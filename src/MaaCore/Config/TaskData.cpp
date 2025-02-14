#include "TaskData.h"

#include <algorithm>
#include <meojson/json.hpp>
#include <regex>

#ifdef ASST_DEBUG
#include <queue>
#endif

#include "Common/AsstTypes.h"
#include "GeneralConfig.h"
#include "TaskData/TaskDataSymbolStream.h"
#include "TaskData/TaskDataTypes.h"
#include "TemplResource.h"
#include "Utils/JsonMisc.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Ranges.hpp"
#include "Utils/StringMisc.hpp"

const std::unordered_set<std::string>& asst::TaskData::get_templ_required() const noexcept
{
    return m_templ_required;
}

asst::TaskDerivedConstPtr asst::TaskData::get_raw(std::string_view name)
{
    if (!generate_raw_task_and_base(name, true)) [[unlikely]] {
        return nullptr;
    }

    if (auto it = m_raw_all_tasks_info.find(name); it != m_raw_all_tasks_info.cend()) {
        return it->second;
    }

    // `@` 前面的字符长度
    size_t name_len = name.find('@');
    if (name_len == std::string_view::npos) [[unlikely]] {
        return nullptr;
    }

    return nullptr;
}

asst::TaskPtr asst::TaskData::get(std::string_view name)
{
    // 生成过的任务
    if (auto it = m_all_tasks_info.find(name); it != m_all_tasks_info.cend()) {
        return it->second;
    }

    auto task = generate_task_info(name);
    if (!task) [[unlikely]] {
        return nullptr;
    }

    constexpr size_t MAX_TASKS_SIZE = 65535;
    if (m_all_tasks_info.size() < MAX_TASKS_SIZE) [[likely]] {
        // 保存最终生成的任务，下次查询时可以直接返回
        return insert_or_assign_task(name, task).first->second;
    }
    else {
        // 个数超过上限时不保存，直接返回，防止内存占用过大
        Log.warn("Task count has exceeded the upper limit:", MAX_TASKS_SIZE, "current task:", name);
        return task;
    }
}

bool asst::TaskData::lazy_parse(const json::value& json)
{
    LogTraceFunction;

    if (!json.is_object()) {
        Log.error("parameter json is not a json::object");
        return false;
    }

    for (const auto& [name, task_json] : json.as_object()) {
        std::string_view name_view = task_name_view(name);
        if (task_json.contains("baseTask")) {
            // 直接声明 baseTask 的任务不继承同名任务参数而是直接覆盖
            m_json_all_tasks_info[name_view] = task_json.as_object();
            std::string base_task = task_json.get("baseTask", "");
#ifdef ASST_DEBUG
            if (base_task.empty()) {
                Log.error("Task", name, "has empty baseTask");
            }
#endif
            if (base_task == "#none") {
                m_json_all_tasks_info[name_view].erase("baseTask");
            }
            continue;
        }
        if (!m_json_all_tasks_info.contains(name_view)) {
            m_json_all_tasks_info.emplace(name_view, task_json.as_object());
            continue;
        }
        for (const auto& [key, value] : task_json.as_object()) {
            m_json_all_tasks_info[name_view][key] = value;
        }
    }

    clear_tasks();

#ifdef ASST_DEBUG
    {
        LogTraceScope("syntax_check");
        bool validity = true;
        std::queue<std::string_view> task_queue;
        std::unordered_set<std::string_view> checking_task_set;

        for (std::string_view name : m_json_all_tasks_info | views::keys) {
            m_task_status[name] = ToBeGenerate;
            task_queue.push(name);
            checking_task_set.insert(name);
        }

        for (const auto& [name, task_json] : m_json_all_tasks_info) [[likely]] {
            validity &= syntax_check(name, task_json);
        }

        const size_t MAX_CHECKING_SIZE = 10000;
        while (!task_queue.empty() && checking_task_set.size() <= MAX_CHECKING_SIZE) {
            std::string_view name = task_queue.front();
            task_queue.pop();
            auto task = get(name);
            if (task == nullptr) [[unlikely]] {
                Log.error("Task", name, "not successfully generated");
                validity = false;
                continue;
            }
            auto check_tasklist =
                [&](const TaskList& task_list, std::string_view list_type, bool enable_justreturn_check = false) {
                    std::unordered_set<std::string_view> tasks_set {};
                    std::string justreturn_task_name = "";
                    for (const std::string& task_name : task_list) {
                        if (tasks_set.contains(task_name)) [[unlikely]] {
                            continue;
                        }
                        // 检查是否有 JustReturn 任务不是最后一个任务
                        if (enable_justreturn_check && !justreturn_task_name.empty()) [[unlikely]] {
                            Log.error(
                                (std::string(name) += "->") += list_type,
                                "has a not-final JustReturn task:",
                                justreturn_task_name);
                            justreturn_task_name = "";
                            validity = false;
                        }

                        if (auto ptr = get(task_name); ptr == nullptr) [[unlikely]] {
                            Log.error(task_name, "in", (std::string(name) += "->") += list_type, "is null");
                            validity = false;
                            continue;
                        }
                        else if (ptr->algorithm == AlgorithmType::JustReturn) {
                            justreturn_task_name = ptr->name;
                        }

                        if (!checking_task_set.contains(task_name)) {
                            task_queue.emplace(task_name_view(task_name));
                            checking_task_set.emplace(task_name_view(task_name));
                        }
                        tasks_set.emplace(task_name_view(task_name));
                    }

                    return true;
                };
            bool enable_justreturn_check = true;
            if (name.ends_with("LoadingText") || name.ends_with("LoadingIcon")) {
                // 放宽对 Loading 类任务的限制，不然 JustReturn 的任务如果本身有 JR 的 next，就不能 @Loading 了
                enable_justreturn_check = false;
            }
            check_tasklist(task->next, "next", enable_justreturn_check);
            check_tasklist(task->sub, "sub");
            check_tasklist(task->exceeded_next, "exceeded_next", enable_justreturn_check);
            check_tasklist(task->on_error_next, "on_error_next", enable_justreturn_check);
            check_tasklist(task->reduce_other_times, "reduce_other_times");

            static const std::unordered_set count_methods { MatchMethod::RGBCount, MatchMethod::HSVCount };
            if (auto match_task = std::dynamic_pointer_cast<MatchTaskInfo>(task);
                task->algorithm == AlgorithmType::MatchTemplate &&
                ranges::find_if(match_task->methods, [&](MatchMethod m) { return count_methods.contains(m); }) !=
                    match_task->methods.cend() &&
                match_task->color_scales.empty()) {
                // RGBCount 和 HSVCount 必须有 color_scales
                Log.error("Task", name, "with Count method has empty color_scales");
                validity = false;
            }
            // 用于解决 a8d68dd72df6eef1d2f8feed3883299922ec1a17 类似的潜在regex非法问题
            if (auto ocr_task = std::dynamic_pointer_cast<OcrTaskInfo>(task);
                task->algorithm == AlgorithmType::OcrDetect) {
                for (const auto& [regex, new_str] : ocr_task->replace_map) {
                    try {
                        std::regex _(regex);
                    }
                    catch (const std::regex_error& e) {
                        Log.error("Task", name, "has invalid regex:", regex, ":", e.what());
                        validity = false;
                        break;
                    }
                }
            }
        }
        if (checking_task_set.size() > MAX_CHECKING_SIZE) {
            // 生成超出上限一般是出现了会导致无限隐式生成的任务。比如 "#self@LoadingText". 这里给个警告.
            Log.warn("Generating exceeded limit when syntax_check.");
        }
        else {
            Log.trace(checking_task_set.size(), "tasks checked.");
        }
        clear_tasks();
        if (!validity) {
            return false;
        }
    }
#endif

    return true;
}

bool asst::TaskData::parse(const json::value& json)
{
    LogTraceFunction;

    if (!lazy_parse(json)) {
        return false;
    }

    // 本来重构之后完全支持惰性加载，但是发现模板图片不支持（
    for (std::string_view name : m_json_all_tasks_info | views::keys) {
        generate_task_info(name);
    }

    return true;
}

void asst::TaskData::clear_tasks()
{
    // 注意：这会导致已经通过 get 获取的任务指针内容不会更新
    // 即运行期修改对已经获取的任务指针无效，但是不会导致崩溃；要想更新，需要重新获取任务指针
    m_all_tasks_info.clear();
    m_raw_all_tasks_info.clear();
    for (std::string_view name : m_json_all_tasks_info | views::keys) {
        m_task_status[task_name_view(name)] = ToBeGenerate;
    }
}

void asst::TaskData::set_task_base(const std::string_view task_name, std::string base_task_name)
{
    m_json_all_tasks_info[task_name_view(task_name)]["baseTask"] = std::move(base_task_name);
    clear_tasks();
}

bool asst::TaskData::generate_raw_task_info(
    std::string_view name,
    std::string_view prefix,
    std::string_view base,
    const json::value& json,
    TaskDerivedType type)
{
    TaskPipelineConstPtr base_task = base.empty() ? nullptr : get_raw(base);
    if (base_task == nullptr) {
        base_task = default_task_info_ptr;
        prefix = "";
    }

    TaskDerivedPtr task = std::make_shared<TaskDerivedInfo>();
    task->name = name;
    task->type = type;
    task->base = base;
    task->prefix = prefix;
    utils::get_and_check_value_or(name, json, "next", task->next, [&]() {
        return append_prefix(base_task->next, prefix);
    });
    utils::get_and_check_value_or(name, json, "sub", task->sub, [&]() {
        return append_prefix(base_task->sub, prefix);
    });
    utils::get_and_check_value_or(name, json, "exceededNext", task->exceeded_next, [&]() {
        return append_prefix(base_task->exceeded_next, prefix);
    });
    utils::get_and_check_value_or(name, json, "onErrorNext", task->on_error_next, [&]() {
        return append_prefix(base_task->on_error_next, prefix);
    });
    utils::get_and_check_value_or(name, json, "reduceOtherTimes", task->reduce_other_times, [&]() {
        return append_prefix(base_task->reduce_other_times, prefix);
    });
    m_task_status[task_name_view(name)] = NotToBeGenerate;

    // 保存虚任务未展开的任务
    insert_or_assign_raw_task(name, task);
    return true;
}

// name: 待生成的任务名
// must_true: 必须有名字为 name 的资源
// allow_implicit: 允许隐式生成（解决 A@B@LoadingText 时 B 不存在的问题）
bool asst::TaskData::generate_raw_task_and_base(std::string_view name, bool must_true, bool allow_implicit)
{
    switch (m_task_status[task_name_view(name)]) {
    case NotToBeGenerate:
        // 已经显式生成
        if (m_raw_all_tasks_info.contains(name)) {
            return true;
        }

        if (!allow_implicit) {
            return false;
        }

        // 隐式生成的资源
        for (size_t p = name.find('@'); p != std::string::npos; p = name.find('@', p + 1)) {
            if (generate_raw_task_and_base(name.substr(p + 1), false, false)) {
                // 隐式 TemplateTask
                generate_raw_task_info(name, name.substr(0, p), name.substr(p + 1), {}, TaskDerivedType::Implicit);
                return true;
            }
        }
        m_task_status[name] = NotExists;

        [[fallthrough]];
    case NotExists:
        if (must_true) {
            Log.error("Unknown task:", name);
        }
        // 不一定必须有名字为 name 的资源，例如 Roguelike@Abandon 不必有 Abandon.
        return false;
    case ToBeGenerate: {
        if (!m_json_all_tasks_info.contains(name)) [[unlikely]] {
            // 这段正常情况来说是不可能的，除非有 string_view 引用失效
            Log.error("Unexcepted ToBeGenerate task:", name);
            return false;
        }

        m_task_status[name] = Generating;

        const json::value& task_json = m_json_all_tasks_info.at(name);

        // BaseTask
        if (auto opt = task_json.find<std::string>("baseTask")) {
            std::string base = opt.value();
            return generate_raw_task_and_base(base, must_true) &&
                   generate_raw_task_info(name, "", base, task_json, TaskDerivedType::BaseTask);
        }

        // TemplateTask
        for (size_t p = name.find('@'); p != std::string::npos; p = name.find('@', p + 1)) {
            if (std::string_view base = name.substr(p + 1); generate_raw_task_and_base(base, false, false)) {
                return generate_raw_task_info(name, name.substr(0, p), base, task_json, TaskDerivedType::Template);
            }
        }

        return generate_raw_task_info(name, "", "", task_json, TaskDerivedType::Raw);
    }
    [[unlikely]] case Generating:
        Log.error("Task", name, "is generated cyclically");
        return false;
    [[unlikely]] default:
        Log.error("Task", name, "has unknown status");
        return false;
    }
}

asst::TaskPtr asst::TaskData::generate_task_info(std::string_view name)
{
    auto raw = get_raw(name);
    if (!raw) [[unlikely]] {
        Log.error("Task", name, "not found");
        return nullptr;
    }

    auto json_it = m_json_all_tasks_info.find(name);
    const json::value& json = json_it == m_json_all_tasks_info.cend() ? json::value {} : json_it->second;
    if (raw->type == TaskDerivedType::Raw && json_it == m_json_all_tasks_info.cend()) [[unlikely]] {
        Log.error("Task", name, "of type Raw has no json");
        return nullptr;
    }
    if (raw->type == TaskDerivedType::Raw && !raw->base.empty()) [[unlikely]] {
        Log.error("Task", name, "of type Raw has base", raw->base);
        return nullptr;
    }
    if (raw->type != TaskDerivedType::Raw && raw->base.empty()) [[unlikely]] {
        Log.error("Task", name, "of type", enum_to_string(raw->type), "has no base");
        return nullptr;
    }
    if ((raw->type == TaskDerivedType::Implicit || raw->type == TaskDerivedType::Template) && raw->prefix.empty())
        [[unlikely]] {
        Log.error("Task", name, "of type", enum_to_string(raw->type), "has no prefix");
        return nullptr;
    }
    if ((raw->type == TaskDerivedType::Raw || raw->type == TaskDerivedType::BaseTask) && !raw->prefix.empty())
        [[unlikely]] {
        Log.error("Task", name, "of type", enum_to_string(raw->type), "has prefix", raw->prefix);
        return nullptr;
    }

    TaskConstPtr base = default_task_info_ptr;
    if (!raw->base.empty()) {
        base = get(raw->base);
        if (!base) [[unlikely]] {
            Log.error("Base task", raw->base, "of task", name, "not found");
            return nullptr;
        }
    }

    // 获取 algorithm 并按照 algorithm 生成 TaskInfo
    AlgorithmType algorithm;
    utils::get_and_check_value_or(name, json, "algorithm", algorithm, base->algorithm);
    TaskPtr task = nullptr;
    switch (algorithm) {
    case AlgorithmType::MatchTemplate:
        task = generate_match_task_info(name, json, std::dynamic_pointer_cast<const MatchTaskInfo>(base), raw->type);
        break;
    case AlgorithmType::OcrDetect:
        task = generate_ocr_task_info(name, json, std::dynamic_pointer_cast<const OcrTaskInfo>(base));
        break;
    case AlgorithmType::JustReturn:
        task = std::make_shared<TaskInfo>();
        break;
    default:
        Log.error("Unknown algorithm in task", name);
        return nullptr;
    }

    if (task == nullptr) {
        return nullptr;
    }

#define ASST_TASKDATA_GET_VALUE_OR(key, value) utils::get_and_check_value_or(name, json, key, task->value, base->value)
#define ASST_TASKDATA_GET_VALUE_OR_LAZY(key, value, m)                                         \
    utils::get_value_or(name, json, key, task->value, raw->value);                             \
    if (auto opt = compile_tasklist(task->value, name, m); !opt) [[unlikely]] {                \
        Log.error("Generate task_list", std::string(name) + "->" key, "failed.", opt.error()); \
        return nullptr;                                                                        \
    }                                                                                          \
    else {                                                                                     \
        task->value = std::move((*opt).tasks);                                                 \
    }

    ASST_TASKDATA_GET_VALUE_OR("action", action);
    ASST_TASKDATA_GET_VALUE_OR("cache", cache);
    ASST_TASKDATA_GET_VALUE_OR("maxTimes", max_times);
    ASST_TASKDATA_GET_VALUE_OR("preDelay", pre_delay);
    ASST_TASKDATA_GET_VALUE_OR("postDelay", post_delay);
    ASST_TASKDATA_GET_VALUE_OR("roi", roi);
    ASST_TASKDATA_GET_VALUE_OR("subErrorIgnored", sub_error_ignored);
    ASST_TASKDATA_GET_VALUE_OR("rectMove", rect_move);
    ASST_TASKDATA_GET_VALUE_OR("specificRect", specific_rect);
    ASST_TASKDATA_GET_VALUE_OR("specialParams", special_params);
    ASST_TASKDATA_GET_VALUE_OR("inputText", input_text);

    // 展开五个任务列表中的虚任务
    ASST_TASKDATA_GET_VALUE_OR_LAZY("next", next, false);
    ASST_TASKDATA_GET_VALUE_OR_LAZY("sub", sub, true);
    ASST_TASKDATA_GET_VALUE_OR_LAZY("exceededNext", exceeded_next, false);
    ASST_TASKDATA_GET_VALUE_OR_LAZY("onErrorNext", on_error_next, false);
    ASST_TASKDATA_GET_VALUE_OR_LAZY("reduceOtherTimes", reduce_other_times, true);

#undef ASST_TASKDATA_GET_VALUE_OR
#undef ASST_TASKDATA_GET_VALUE_OR_LAZY

#ifdef ASST_DEBUG
    if (!json.contains("roi") && base == default_task_info_ptr // 无 base 任务
        && algorithm != asst::AlgorithmType::JustReturn        // 非 JustReturn
        && !task->next.empty()                                 // 有 next
        && (algorithm != asst::AlgorithmType::MatchTemplate    // template 不是 empty
            || std::dynamic_pointer_cast<const MatchTaskInfo>(task)->templ_names !=
                   std::vector<std::string> { "empty.png" })) {
        // 符合上述条件时，我们认为此时的隐式全屏 roi 不是期望行为，给个警告
        Log.warn("Task", name, "has implicit fullscreen roi.");
    }

    // Debug 模式下检查 roi 是否超出边界
    if (auto [x, y, w, h] = task->roi; x + w > WindowWidthDefault || y + h > WindowHeightDefault) {
        Log.warn(name, "roi is out of bounds");
    }
#endif
    task->algorithm = algorithm;
    task->name = name;

    return task;
}

asst::TaskPtr asst::TaskData::generate_match_task_info(
    std::string_view name,
    const json::value& task_json,
    MatchTaskConstPtr default_ptr,
    TaskDerivedType derived_type)
{
    if (default_ptr == nullptr) {
        default_ptr = default_match_task_info_ptr;
    }
    auto match_task_info_ptr = std::make_shared<MatchTaskInfo>();
    if (!utils::get_and_check_value_or(name, task_json, "template", match_task_info_ptr->templ_names, [&]() {
            return derived_type == TaskDerivedType::Implicit // 隐式 Template Task 时继承，其它时默认值使用任务名
                       ? default_ptr->templ_names
                       : std::vector { std::string(name) + ".png" };
        })) {
        return nullptr;
    }

    m_templ_required.insert(match_task_info_ptr->templ_names.begin(), match_task_info_ptr->templ_names.end());

    // 其余若留空则继承模板任务

    auto threshold_opt = task_json.find("templThreshold");
    if (!threshold_opt) {
        match_task_info_ptr->templ_thresholds = default_ptr->templ_thresholds;
        match_task_info_ptr->templ_thresholds.resize(
            match_task_info_ptr->templ_names.size(),
            default_ptr->templ_thresholds.back());
    }
    else if (threshold_opt->is_number()) {
        // 单个数值时，所有模板都使用这个阈值
        match_task_info_ptr->templ_thresholds.resize(
            match_task_info_ptr->templ_names.size(),
            threshold_opt->as_double());
    }
    else if (threshold_opt->is_array()) {
        ranges::copy(
            threshold_opt->as_array() | views::transform(&json::value::as_double),
            std::back_inserter(match_task_info_ptr->templ_thresholds));
    }
    else {
        Log.error("Invalid templThreshold type in task", name);
        return nullptr;
    }

    if (match_task_info_ptr->templ_names.size() != match_task_info_ptr->templ_thresholds.size()) {
        Log.error("Template count and templThreshold count not match in task", name);
        return nullptr;
    }

    if (match_task_info_ptr->templ_names.size() == 0 || match_task_info_ptr->templ_thresholds.size() == 0) {
        Log.error("Template or templThreshold is empty in task", name);
        return nullptr;
    }

    auto method_opt = task_json.find("method");
    if (!method_opt) {
        match_task_info_ptr->methods = default_ptr->methods;
        match_task_info_ptr->methods.resize(match_task_info_ptr->templ_names.size(), default_ptr->methods.back());
    }
    else if (method_opt->is_string()) {
        // 单个数值时，所有模板都使用这个阈值
        match_task_info_ptr->methods.resize(
            match_task_info_ptr->templ_names.size(),
            get_match_method(method_opt->as_string()));
    }
    else if (method_opt->is_array()) {
        ranges::copy(
            method_opt->as_array() | views::transform(&json::value::as_string) | views::transform(&get_match_method),
            std::back_inserter(match_task_info_ptr->methods));
    }
    else {
        Log.error("Invalid method type in task", name);
        return nullptr;
    }

    if (ranges::find(match_task_info_ptr->methods, MatchMethod::Invalid) != match_task_info_ptr->methods.end()) {
        Log.error("Invalid method in task", name);
        return nullptr;
    }

    if (match_task_info_ptr->templ_names.size() != match_task_info_ptr->methods.size()) {
        Log.error("Template count and method count not match in task", name);
        return nullptr;
    }

    if (match_task_info_ptr->templ_names.size() == 0 || match_task_info_ptr->methods.size() == 0) {
        Log.error("Template or method is empty in task", name);
        return nullptr;
    }

    if (auto mask_opt = task_json.find("maskRange"); !mask_opt) {
        match_task_info_ptr->mask_ranges = default_ptr->mask_ranges;
    }
    else if (!mask_opt->is_array()) {
        Log.error("Invalid mask_range type in task", name, ", should be `array<int, 2>`");
        return nullptr;
    }
    else if (auto mask_array = mask_opt->as_array();
             mask_array.size() == 2 && mask_array[0].is_number() && mask_array[1].is_number()) {
        match_task_info_ptr->mask_ranges.emplace_back(
            MatchTaskInfo::GrayRange { mask_array[0].as_integer(), mask_array[1].as_integer() });
    }
    else {
        Log.error("Invalid mask_range in task", name);
        return nullptr;
    }

    if (auto color_opt = task_json.find("colorScales"); !color_opt) {
        match_task_info_ptr->color_scales = default_ptr->color_scales;
    }
    else if (!color_opt->is_array()) {
        Log.error("Invalid color_scales type in task", name);
        return nullptr;
    }
    else if (auto color_array = color_opt->as_array();
             color_array.size() == 2 && color_array[0].is_number() && color_array[1].is_number()) {
        // gray scale, color_array is array<int, 2>
        Log.debug("Deprecated GrayRange color_scales in task", name, ", should be `list<pair<int, int>>`");
        match_task_info_ptr->color_scales.emplace_back(
            MatchTaskInfo::GrayRange { color_array[0].as_integer(), color_array[1].as_integer() });
    }
    else {
        /*  [
                [[0, 0, 0], [0, 0, 255]],
                [[0, 0, 0], [0, 255, 0]],
                [1, 255]
            ]
        */
        match_task_info_ptr->color_scales.clear();
        for (const auto& color_array_item : color_array) {
            if (!color_array_item.is_array()) {
                Log.error("Invalid color_range in task", name);
                return nullptr;
            }
            const auto& color_range = color_array_item.as_array();
            if (color_range.size() != 2) { // lower & upper, 2 elements
                Log.error(
                    "Invalid color_range in task",
                    name,
                    ", should have 2 elements (lower & upper) in each array");
                return nullptr;
            }

            const auto& lower_item = color_range[0];
            const auto& upper_item = color_range[1];

            // gray scale, color_array_item is array<int, 2> (recommended)
            if (lower_item.is_number() && upper_item.is_number()) {
                match_task_info_ptr->color_scales.emplace_back(
                    MatchTaskInfo::GrayRange { lower_item.as_integer(), upper_item.as_integer() });
                continue;
            }

            if (!lower_item.is_array() || !upper_item.is_array()) {
                Log.error("Invalid color_range in task", name);
                return nullptr;
            }

            // color, color_array_item is array<array<int, 3>, 2>
            const auto& lower = lower_item.as_array();
            const auto& upper = upper_item.as_array();

            if (!ranges::all_of(std::array { lower, upper } | views::join, &json::value::is_number)) {
                Log.error("Invalid color_range in task", name);
                return nullptr;
            }
            auto lower_number = lower | views::transform(&json::value::as_integer);
            auto upper_number = upper | views::transform(&json::value::as_integer);

            if (lower_number.size() == 1 && upper_number.size() == 1) {
                // gray scale "[..., [[0], [255]], ...]"
                Log.debug("Not recommended GrayRange color_scales in task", name, ", should be `list<pair<int, int>>`");
                match_task_info_ptr->color_scales.emplace_back(
                    MatchTaskInfo::GrayRange { lower_number[0], upper_number[0] });
                continue;
            }
            if (lower_number.size() == 3 && upper_number.size() == 3) {
                // color scale "[..., [[0, 0, 0], [0, 0, 255]], ...]"
                match_task_info_ptr->color_scales.emplace_back(
                    MatchTaskInfo::ColorRange { std::array { lower_number[0], lower_number[1], lower_number[2] },
                                                std::array { upper_number[0], upper_number[1], upper_number[2] } });
                continue;
            }
            Log.error("Invalid color_range in task", name);
            return nullptr;
        }
    }

    utils::get_and_check_value_or(
        name,
        task_json,
        "colorWithClose",
        match_task_info_ptr->color_close,
        default_ptr->color_close);

    return match_task_info_ptr;
}

asst::TaskPtr asst::TaskData::generate_ocr_task_info(
    std::string_view name,
    const json::value& task_json,
    OcrTaskConstPtr default_ptr)
{
    if (default_ptr == nullptr) {
        default_ptr = default_ocr_task_info_ptr;
    }
    auto ocr_task_info_ptr = std::make_shared<OcrTaskInfo>();

    // text 不允许为字符串，必须是字符串数组，不能用 utils::get_value_or
    auto array_opt = task_json.find<json::array>("text");
    ocr_task_info_ptr->text = array_opt ? to_string_list(array_opt.value()) : default_ptr->text;
#ifdef ASST_DEBUG
    if (!array_opt && default_ptr == default_ocr_task_info_ptr) {
        Log.warn("Ocr task", name, "has implicit empty text.");
    }
#endif
    utils::get_and_check_value_or(name, task_json, "fullMatch", ocr_task_info_ptr->full_match, default_ptr->full_match);
    utils::get_and_check_value_or(name, task_json, "isAscii", ocr_task_info_ptr->is_ascii, default_ptr->is_ascii);
    utils::get_and_check_value_or(
        name,
        task_json,
        "withoutDet",
        ocr_task_info_ptr->without_det,
        default_ptr->without_det);
    utils::get_and_check_value_or(
        name,
        task_json,
        "replaceFull",
        ocr_task_info_ptr->replace_full,
        default_ptr->replace_full);
    utils::get_and_check_value_or(
        name,
        task_json,
        "ocrReplace",
        ocr_task_info_ptr->replace_map,
        default_ptr->replace_map);
    return ocr_task_info_ptr;
}

asst::ResultOrError<asst::TaskData::RawCompileResult> asst::TaskData::compile_raw_tasklist(
    const TaskList& raw_tasks,
    std::string_view self_name,
    std::function<TaskDerivedConstPtr(std::string_view)> get_raw,
    bool allow_duplicate)
{
    RawCompileResult ret { .task_changed = false, .symbols = {} };
    std::unordered_set<std::string_view> tasks_set; // 记录任务列表中已有的任务（内容元素与 new_tasks 基本一致）

    for (std::string_view task_expr : raw_tasks) {
        if (task_expr.empty() || (!allow_duplicate && tasks_set.contains(task_expr))) {
            ret.task_changed = true;
            continue;
        }

        TaskDataSymbolStream symbol_stream;
        if (auto opt = symbol_stream.parse(task_expr); opt) [[likely]] {
            ret.task_changed = ret.task_changed || opt.value();
        }
        else {
            return { std::nullopt, std::move(opt.error()) };
        }

        auto opt = symbol_stream.decode(
            [&](const TaskDataSymbol& symbol, const TaskDataSymbol& prefix) -> TaskDataSymbol::SymbolsOrError {
                return TaskDataSymbol::append_prefix(
                    symbol,
                    prefix,
                    self_name,
                    get_raw,
                    [&](const TaskList& raw_or_empty) -> TaskDataSymbol::SymbolsOrError {
                        if (auto opt = compile_raw_tasklist(raw_or_empty, self_name, get_raw, allow_duplicate)) {
                            return opt.value().symbols;
                        }
                        return { std::nullopt, "" };
                    });
            },
            self_name);

        // 记号流处理
        if (!opt) {
            return { std::nullopt, std::move(opt.error()) };
        }

        for (const auto& task : *opt) {
            if (task.name().empty() || (!allow_duplicate && tasks_set.contains(task.name()))) {
                ret.task_changed = true;
                continue;
            }
            tasks_set.emplace(ret.symbols.emplace_back(task).name());
        }

        tasks_set.emplace(task_name_view(task_expr));
    }

    return ret;
}

asst::ResultOrError<asst::TaskData::CompileResult>
    asst::TaskData::compile_tasklist(const TaskList& raw_tasks, std::string_view self_name, bool allow_duplicate)
{
    CompileResult ret { .task_changed = false, .tasks = {} };
    std::vector<TaskDataSymbol> new_symbols;
    if (auto opt = compile_raw_tasklist(
            raw_tasks,
            self_name,
            [&](std::string_view name) { return get_raw(name); },
            allow_duplicate);
        !opt) {
        return { std::nullopt, std::move(opt.error()) };
    }
    else {
        new_symbols = std::move(opt.value().symbols);
        ret.task_changed = opt.value().task_changed;
    }
    std::unordered_set<std::string_view> tasks_set;
    for (const auto& task : new_symbols) {
        std::string_view task_name;
        if (task == TaskDataSymbol::SharpSelf) {
            task_name = self_name;
            ret.task_changed = true;
        }
        else if (task.is_name()) {
            task_name = task.name();
        }
        else {
            ret.task_changed = true;
            continue;
        }
        if (task_name.empty()) {
            Log.error("Empty task name in", self_name);
            ret.task_changed = true;
            continue;
        }
        if (!allow_duplicate && tasks_set.contains(task_name)) {
            ret.task_changed = true;
            continue;
        }
        ret.tasks.emplace_back(task_name_view(task_name));
        tasks_set.emplace(task_name);
    }

    return ret;
}

std::string asst::TaskData::append_prefix(std::string_view task_name, std::string_view task_prefix)
{
    if (task_prefix.ends_with('@')) [[unlikely]] {
        task_prefix.remove_suffix(1);
    }
    if (task_prefix.empty()) [[unlikely]] {
        return std::string(task_name);
    }
    if (task_name.empty()) {
        return std::string(task_prefix);
    }
    if (task_name.starts_with('#')) {
        return std::string(task_prefix) + std::string(task_name);
    }
    return std::string(task_prefix) + '@' + std::string(task_name);
}

asst::TaskList asst::TaskData::append_prefix(const TaskList& base_task_list, std::string_view task_prefix)
{
    if (task_prefix.ends_with('@')) [[unlikely]] {
        task_prefix.remove_suffix(1);
    }
    if (task_prefix.empty()) {
        return base_task_list;
    }
    TaskList task_list = {};
    for (const std::string& base : base_task_list) {
        std::string_view base_view = base;
        size_t l = 0;
        bool has_same_prefix = false;
        // base 任务里已经存在相同前缀，则不加前缀
        // https://github.com/MaaAssistantArknights/MaaAssistantArknights/pull/2116#issuecomment-1270115238
        for (size_t r = base_view.find('@'); r != std::string_view::npos; r = base_view.find('@', l)) {
            if (task_prefix == base_view.substr(l, r - l)) [[unlikely]] {
                has_same_prefix = true;
                break;
            }
            l = r + 1;
        }
        task_list.emplace_back(has_same_prefix ? base : append_prefix(base_view, task_prefix));
    }
    return task_list;
}

asst::MatchTaskConstPtr asst::TaskData::_default_match_task_info()
{
    auto match_task_info_ptr = std::make_shared<MatchTaskInfo>();
    match_task_info_ptr->templ_names = { "__INVALID__" };
    match_task_info_ptr->templ_thresholds = std::vector<double> { TemplThresholdDefault };
    match_task_info_ptr->methods = std::vector<MatchMethod> { MatchMethod::Ccoeff };
    match_task_info_ptr->mask_ranges = {};
    match_task_info_ptr->color_scales = {};
    match_task_info_ptr->color_close = true;

    return match_task_info_ptr;
}

asst::OcrTaskConstPtr asst::TaskData::_default_ocr_task_info()
{
    auto ocr_task_info_ptr = std::make_shared<OcrTaskInfo>();
    ocr_task_info_ptr->full_match = false;
    ocr_task_info_ptr->is_ascii = false;
    ocr_task_info_ptr->without_det = false;
    ocr_task_info_ptr->replace_full = false;

    return ocr_task_info_ptr;
}

asst::TaskConstPtr asst::TaskData::_default_task_info()
{
    auto task_info_ptr = std::make_shared<TaskInfo>();
    task_info_ptr->algorithm = AlgorithmType::MatchTemplate;
    task_info_ptr->action = ProcessTaskAction::DoNothing;
    task_info_ptr->cache = false;
    task_info_ptr->max_times = INT_MAX;
    task_info_ptr->pre_delay = 0;
    task_info_ptr->post_delay = 0;
    task_info_ptr->roi = Rect();
    task_info_ptr->sub_error_ignored = false;
    task_info_ptr->rect_move = Rect();
    task_info_ptr->specific_rect = Rect();

    return task_info_ptr;
}

#ifdef ASST_DEBUG
// 为了解决类似 beddc7c828126c678391e0b4da288db6d2c2d58a 导致的问题，加载的时候做一个语法检查
// 主要是处理是否包含未知键值的问题
bool asst::TaskData::syntax_check(std::string_view task_name, const json::value& task_json)
{
    // 以下按字典序排序
    // clang-format off
    static const std::unordered_map<AlgorithmType, std::unordered_set<std::string>> allowed_key_under_algorithm = {
        { AlgorithmType::Invalid, {} },
        { AlgorithmType::MatchTemplate,
          {
              // common
              "action",        "algorithm",     "baseTask",        "exceededNext",   "maxTimes",
              "next",          "onErrorNext",   "postDelay",       "preDelay",       "reduceOtherTimes",
              "specialParams", "sub",           "subErrorIgnored",

              // specific
              "cache",         "colorScales",   "colorWithClose",  "maskRange",      "method",
              "rectMove",      "roi",           "specialParams",   "templThreshold", "template",
          } },
        { AlgorithmType::OcrDetect,
          {
              // common
              "action",        "algorithm",   "baseTask",        "exceededNext", "maxTimes",
              "next",          "onErrorNext", "postDelay",       "preDelay",     "reduceOtherTimes",
              "specialParams", "sub",         "subErrorIgnored",

              // specific
              "cache",         "fullMatch",   "isAscii",         "ocrReplace",   "rectMove",
              "replaceFull",   "roi",         "text",            "withoutDet",
          } },
        { AlgorithmType::JustReturn,
          {
              // common
              "action",        "algorithm",   "baseTask",        "exceededNext", "maxTimes",
              "next",          "onErrorNext", "postDelay",       "preDelay",     "reduceOtherTimes",
              "specialParams", "sub",         "subErrorIgnored",

              // specific
              "inputText"
          } },
    };
    // clang-format on

    static const std::unordered_map<ProcessTaskAction, std::unordered_set<std::string>> allowed_key_under_action = {
        { ProcessTaskAction::ClickRect, { "specificRect" } },
        { ProcessTaskAction::Swipe, { "specificRect", "rectMove" } },
        { ProcessTaskAction::Input, { "inputText" } },
    };

    auto is_doc = [&](std::string_view key) {
        return key.find("Doc") != std::string_view::npos || key.find("doc") != std::string_view::npos;
    };

    // 兜底策略，如果某个 key ("xxx") 不符合规范（可能是代码中使用到的参数，而不是任务流程）
    // 需要加一个注释 ("xxx_Doc") 就能过 syntax_check.
    auto has_doc = [&](const std::string& key) -> bool {
        return task_json.find(key + "_Doc") || task_json.find(key + "_doc");
    };

    if (!task_json.is_object()) {
        Log.error(task_name, "is not a json object.");
        return false;
    }

    bool validity = true;
    auto task_ptr = get(task_name);
    if (task_ptr == nullptr) {
        Log.error("TaskData::syntax_check | Task", task_name, "has not been generated.");
        return false;
    }

    // 获取 algorithm
    auto algorithm = task_ptr->algorithm;
    if (algorithm == AlgorithmType::Invalid) [[unlikely]] {
        Log.error(task_name, "has unknown algorithm.");
        validity = false;
    }

    // 获取 action
    auto action = task_ptr->action;
    if (action == ProcessTaskAction::Invalid) [[unlikely]] {
        Log.error(task_name, "has unknown action.");
        validity = false;
    }

    std::unordered_set<std::string> allowed_key {};
    if (allowed_key_under_algorithm.contains(algorithm)) {
        decltype(allowed_key) tmp = allowed_key_under_algorithm.at(algorithm);
        allowed_key.merge(tmp);
    }
    if (allowed_key_under_action.contains(action)) {
        decltype(allowed_key) tmp = allowed_key_under_action.at(action);
        allowed_key.merge(tmp);
    }

    for (const auto& name : task_json.as_object() | views::keys) {
        if (!allowed_key.contains(name) && !is_doc(name) && !has_doc(name)) {
            Log.error(task_name, "has unknown key:", name);
            validity = false;
        }
    }
    return validity;
}
#endif
