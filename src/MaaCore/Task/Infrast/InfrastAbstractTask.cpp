#include "InfrastAbstractTask.h"

#include <algorithm>
#include <regex>
#include <utility>

#include "Common/AsstMsg.h"
#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Status.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Utils/Ranges.hpp"
#include "Vision/Infrast/InfrastFacilityImageAnalyzer.h"
#include "Vision/Infrast/InfrastOperImageAnalyzer.h"
#include "Vision/Matcher.h"
#include "Vision/OCRer.h"
#include "Vision/RegionOCRer.h"

asst::InfrastAbstractTask::InfrastAbstractTask(
    const AsstCallback& callback,
    Assistant* inst,
    std::string_view task_chain) :
    AbstractTask(callback, inst, task_chain)
{
    m_retry_times = TaskRetryTimes;
}

asst::InfrastAbstractTask& asst::InfrastAbstractTask::set_mood_threshold(double mood_thres) noexcept
{
    m_mood_threshold = mood_thres;
    return *this;
}

json::value asst::InfrastAbstractTask::basic_info() const
{
    json::value info = AbstractTask::basic_info();
    auto& details = info["details"];
    details["facility"] = facility_name();
    details["index"] = m_cur_facility_index;
    return info;
}

std::string asst::InfrastAbstractTask::facility_name() const
{
    if (m_facility_name_cache.empty()) {
        std::string class_name = typeid(*this).name();
        // typeid.name() 结果可能和编译器有关，所以这里使用正则尽可能保证结果正确。
        // 但还是不能完全保证，如果不行的话建议 override
        std::regex regex("Infrast(.*)Task");
        std::smatch match_obj;
        if (std::regex_search(class_name, match_obj, regex)) {
            m_facility_name_cache = match_obj[1].str();
        }
        else {
            m_facility_name_cache = class_name;
        }
    }
    return m_facility_name_cache;
}

void asst::InfrastAbstractTask::set_custom_config(infrast::CustomFacilityConfig config) noexcept
{
    m_custom_config = std::move(config);
    m_is_custom = true;
}

void asst::InfrastAbstractTask::clear_custom_config() noexcept
{
    m_is_custom = false;
    m_custom_config.clear();
}

asst::infrast::CustomRoomConfig& asst::InfrastAbstractTask::current_room_config()
{
    static infrast::CustomRoomConfig empty;
    if (!m_is_custom) [[unlikely]] {
        Log.warn(__FUNCTION__, "custom is not enabled");
        return empty;
    }

    if (static_cast<size_t>(m_cur_facility_index) < m_custom_config.size()) [[likely]] {
        return m_custom_config.at(m_cur_facility_index);
    }
    else {
        Log.warn(__FUNCTION__, "index out of range:", m_cur_facility_index, m_custom_config.size());
        return empty;
    }
}

bool asst::InfrastAbstractTask::match_operator_groups()
{
    current_room_config().names.clear();
    int swipe_times = 0;
    bool pre_result_no_changes = false, retried = false;

    auto opers = get_available_oper_for_group();
    if (opers.size() == 0) {
        Log.info(__FUNCTION__, "available operator for group is empty");
        std::vector<std::string> temp, pre_temp;
        while (true) {
            if (need_exit()) {
                return false;
            }
            temp.clear();
            if (!get_opers(temp, m_mood_threshold)) {
                return false;
            }
            if (pre_temp == temp) {
                if (pre_result_no_changes) {
                    Log.warn("partial result is not changed, reset the page");
                    if (retried) {
                        Log.error("already retried");
                        break;
                    }
                    swipe_to_the_left_of_operlist(swipe_times + 1);
                    swipe_times = 0;
                    retried = true;
                }
                else {
                    pre_result_no_changes = true;
                }
            }
            else {
                pre_result_no_changes = false;
            }
            opers.insert(temp.begin(), temp.end());
            pre_temp = temp;
            swipe_of_operlist();
            swipe_times++;
        }
    }
    swipe_to_the_left_of_operlist(swipe_times + 1);
    swipe_times = 0;
    Log.info(__FUNCTION__, "available operators for group size:", opers.size());
    // 筛选第一个满足要求的干员组
    for (const auto& oper_group_pair : current_room_config().operator_groups) {
        if (ranges::all_of(oper_group_pair.second, [opers](const std::string& oper) { return opers.contains(oper); })) {
            ranges::for_each(oper_group_pair.second, [&opers](const std::string& oper) { opers.erase(oper); });
            current_room_config().names.insert(
                current_room_config().names.end(),
                oper_group_pair.second.begin(),
                oper_group_pair.second.end());

            json::value sanity_info = basic_info_with_what("CustomInfrastRoomGroupsMatch");
            sanity_info["details"]["group"] = oper_group_pair.first;
            callback(AsstMsg::SubTaskExtraInfo, sanity_info);
            break;
        }
    }
    set_available_oper_for_group(std::move(opers));
    // 匹配失败，无分组可用
    if (current_room_config().names.empty() && !current_room_config().operator_groups.empty()) {
        json::value info = basic_info_with_what("CustomInfrastRoomGroupsMatchFailed");
        std::vector<std::string> names;
        ranges::for_each(
            current_room_config().operator_groups,
            [&names](std::pair<const std::string, std::vector<std::string>>& pair) { names.emplace_back(pair.first); });
        info["details"]["groups"] = json::array(std::move(names));
        callback(AsstMsg::SubTaskExtraInfo, info);
        return false;
    }
    return true;
}

std::set<std::string> asst::InfrastAbstractTask::get_available_oper_for_group()
{
    std::set<std::string> opers;
    const auto& str = status()->get_str(Status::InfrastAvailableOpersForGroup);
    if (!str) {
        return opers;
    }
    auto json_array = json::array(json::parse(*str).value_or(json::value(json::array())));
    for (const auto& token : json_array) {
        opers.emplace(token.as_string());
    }
    return opers;
}

void asst::InfrastAbstractTask::set_available_oper_for_group(std::set<std::string> opers)
{
    json::array value;
    for (const auto& oper : opers) {
        value.emplace_back(oper);
    }
    status()->set_str(Status::InfrastAvailableOpersForGroup, value.dumps());
}

bool asst::InfrastAbstractTask::on_run_fails()
{
    LogTraceFunction;

    ProcessTask return_task(*this, { "InfrastBegin" });
    return return_task.run();
}

bool asst::InfrastAbstractTask::enter_facility(int index)
{
    LogTraceFunction;

    if (m_is_custom && static_cast<size_t>(m_cur_facility_index) >= m_custom_config.size()) {
        Log.warn("index out of range:", index, m_custom_config.size());
        return false;
    }

    InfrastFacilityImageAnalyzer analyzer(ctrler()->get_image());
    analyzer.set_to_be_analyzed({ facility_name() });
    if (!analyzer.analyze()) {
        Log.info("result is empty");
        return false;
    }
    Rect rect = analyzer.get_rect(facility_name(), index);
    if (rect.empty()) {
        Log.info("facility index is out of range");
        return false;
    }
    ctrler()->click(rect);
    m_cur_facility_index = index;

    callback(AsstMsg::SubTaskExtraInfo, basic_info_with_what("EnterFacility"));
    sleep(Task.get("InfrastEnterFacility")->post_delay);

    return true;
}

bool asst::InfrastAbstractTask::enter_oper_list_page()
{
    LogTraceFunction;

    ProcessTask task(*this, { "InfrastEnterOperList", "InfrastStationedInfo" });
    return task.run();
}

bool asst::InfrastAbstractTask::is_use_custom_opers()
{
    return m_is_custom && (!current_room_config().names.empty() || !current_room_config().candidates.empty());
}

/// @brief 按技能排序->清空干员->选择定制干员->按指定顺序排序
/// @param is_dorm_order 当前房间是不是宿舍
bool asst::InfrastAbstractTask::swipe_and_select_custom_opers(bool is_dorm_order)
{
    LogTraceFunction;

    auto& room_config = current_room_config();
    auto origin_room_config = room_config;
    {
        json::value cb_info = basic_info_with_what("CustomInfrastRoomOperators");
        auto& details = cb_info["details"];
        details["names"] = json::array(room_config.names);
        details["candidates"] = json::array(room_config.candidates);
        callback(AsstMsg::SubTaskExtraInfo, cb_info);
    }

    if (!is_dorm_order) {
        ProcessTask(*this, { "InfrastOperListTabSkillUnClicked", "Stop" }).run();
    }
    else {
        ProcessTask(*this, { "InfrastOperListTabMoodDoubleClickWhenUnclicked" }).run();
    }

    if (max_num_of_opers() > 1) {
        click_clear_button(); // 先排序后清空，加速干员变化不大时的选择速度
    }

    std::vector<std::string> opers_order = room_config.names;
    opers_order.insert(opers_order.end(), room_config.candidates.cbegin(), room_config.candidates.cend());

    std::vector<std::string> pre_partial_result;
    bool retried = false;
    bool pre_result_no_changes = false;
    int swipe_times = 0;

    while (true) {
        if (need_exit()) {
            return false;
        }
        std::vector<std::string> partial_result;
        // 选择自定义干员，同时输出每一页的干员列表
        if (!select_custom_opers(partial_result)) {
            return false;
        }
        // 选完人 / 名单已空
        if (static_cast<size_t>(room_config.selected) >= max_num_of_opers() ||
            (room_config.names.empty() && room_config.candidates.empty())) {
            break;
        }
        if (partial_result == pre_partial_result) {
            if (pre_result_no_changes) {
                Log.warn("partial result is not changed, reset the page");
                if (retried) {
                    Log.error("already retring");
                    break;
                }
                swipe_to_the_left_of_operlist(swipe_times + 1);
                swipe_times = 0;
                retried = true;
            }
            else {
                pre_result_no_changes = true;
            }
        }
        else {
            pre_result_no_changes = false;
        }
        pre_partial_result = partial_result;
        swipe_of_operlist();
        ++swipe_times;
        // 最后一页会触底反弹，先糊个屎避免一下
        // 总不能有成体系的干员了还没160个人吧）
        if (swipe_times > 20) {
            sleep(500);
        }
    }

    // 先按任意其他的tab排序，游戏会自动把已经选中的人放到最前面
    // 因为后面autofill要按工作状态排序，所以直接按工作状态排序好了
    // 然后滑动到最左边，清空一下，在走后面的识别+按序点击逻辑
    if (is_dorm_order) {
        ProcessTask(*this, { "InfrastOperListTabMoodDoubleClick" }).run();
        sleep(200);
    }
    else {
        ProcessTask(*this, { "InfrastOperListTabWorkStatusUnClicked" }).run();
    }

    if (swipe_times) {
        swipe_to_the_left_of_operlist(swipe_times + 1);
        swipe_times = 0;
    }
    // 如果只选了一个人没必要排序
    if (room_config.sort && room_config.selected > 1) {
        click_clear_button();
        order_opers_selection(opers_order);
    }

    if (!room_config.names.empty() || (!is_dorm_order && !select_opers_review(origin_room_config))) {
        // 复核失败，说明current_room_config与OCR识别是不符的，current_room_config是无效信息，还原到用户原来的配置，重选
        current_room_config() = std::move(origin_room_config);
        return false;
    }

    return true;
}

/// @brief 复核干员选择是否符合期望。如果是自定义的，会检查自定义的人选全了没。
/// @brief 调用该函数前，需保证停留在干员选择页面第一页，且已选干员排在最前面。
/// @param origin_room_config 期望的配置
/// @param num_of_opers_expect 期望选中的人数，空置则按names.size()判断
/// @return 是否符合期望
bool asst::InfrastAbstractTask::select_opers_review(
    infrast::CustomRoomConfig const& origin_room_config,
    size_t num_of_opers_expect)
{
    LogTraceFunction;
    // save_img("debug/");
    auto room_config = origin_room_config;

    const auto image = ctrler()->get_image();
    InfrastOperImageAnalyzer oper_analyzer(image);
    oper_analyzer.set_to_be_calced(
        InfrastOperImageAnalyzer::ToBeCalced::Selected | InfrastOperImageAnalyzer::ToBeCalced::Doing);
    if (!oper_analyzer.analyze()) {
        Log.warn("No oper");
        return false;
    }
    oper_analyzer.sort_by_loc();
    const auto& oper_analyzer_res = oper_analyzer.get_result();
    size_t selected_count =
        ranges::count_if(oper_analyzer_res, [](const infrast::Oper& info) { return info.selected; });
    Log.info(
        "selected_count,config.names.size,num_of_opers_expect = ",
        selected_count,
        ",",
        room_config.names.size(),
        ",",
        num_of_opers_expect);

    if (selected_count < num_of_opers_expect) {
        Log.warn("select opers review fail: unexpected number of selected operators ");
        return false;
    }
    if (facility_name() != "Dorm" && (!m_is_custom || (room_config.names.empty() && room_config.candidates.empty()))) {
        return true;
    }
    if (selected_count < room_config.names.size()) {
        Log.warn("select opers review fail: part of custom operators unselected");
        return false;
    }

    const auto& ocr_replace = Task.get<OcrTaskInfo>("CharsNameOcrReplace");
    for (const auto& oper : oper_analyzer_res) {
        RegionOCRer name_analyzer;
        name_analyzer.set_replace(ocr_replace->replace_map, ocr_replace->replace_full);
        name_analyzer.set_image(oper.name_img);
        name_analyzer.set_bin_expansion(0);
        if (!name_analyzer.analyze()) {
            continue;
        }
        if (!oper.selected) {
            break;
        }

        const std::string& name = name_analyzer.get_result().text;
        if (auto iter = ranges::find(room_config.names, name); iter != room_config.names.end()) {
            Log.info(name, "is in \"operators\"，and is selected");
            room_config.names.erase(iter);
        }
        else { // 备选干员或自动选择，只要不选工作中的干员即可
            if (oper.doing == infrast::Doing::Working) {
                Log.warn("selected operators at work:", name);
                Log.warn("select opers review fail: non-custom configuration, but an operator at work is selected");
                return false;
            }
        }
    }

    if (room_config.names.size()) {
        Log.warn("select opers review fail: part of custom operators unselected.");
        return false;
    }

    Log.info("select opers review passed");
    return true;
}

bool asst::InfrastAbstractTask::select_custom_opers(std::vector<std::string>& partial_result)
{
    LogTraceFunction;

    auto& room_config = current_room_config();
    if (room_config.names.empty() && room_config.candidates.empty()) {
        Log.warn("opers_name is empty");
        return false;
    }

    const auto image = ctrler()->get_image();
    InfrastOperImageAnalyzer oper_analyzer(image);
    oper_analyzer.set_to_be_calced(InfrastOperImageAnalyzer::ToBeCalced::Selected);
    if (!oper_analyzer.analyze()) {
        Log.warn("No oper");
        return false;
    }
    oper_analyzer.sort_by_loc();
    partial_result.clear();

    const auto& ocr_replace = Task.get<OcrTaskInfo>("CharsNameOcrReplace");
    for (const auto& oper : oper_analyzer.get_result()) {
        RegionOCRer name_analyzer;
        name_analyzer.set_replace(ocr_replace->replace_map, ocr_replace->replace_full);
        name_analyzer.set_image(oper.name_img);
        name_analyzer.set_bin_expansion(0);
        if (!name_analyzer.analyze()) {
            continue;
        }
        const std::string& name = name_analyzer.get_result().text;
        partial_result.emplace_back(name);

        if (auto iter = ranges::find(room_config.names, name); iter != room_config.names.end()) {
            room_config.names.erase(iter);
        }
        else if (max_num_of_opers() - room_config.selected > room_config.names.size()) {
            // names中的数量，比剩余的空位多，就可以选备选的
            if (auto candd_iter = ranges::find(room_config.candidates, name);
                candd_iter != room_config.candidates.end()) {
                room_config.candidates.erase(candd_iter);
            }
            else {
                continue;
            }
        }
        else {
            continue;
        }
        if (!oper.selected) {
            ctrler()->click(oper.rect);
        }
        if (static_cast<size_t>(++room_config.selected) >= max_num_of_opers()) {
            break;
        }
    }
    return true;
}

bool asst::InfrastAbstractTask::get_opers(std::vector<std::string>& result, double mood)
{
    LogTraceFunction;

    const auto image = ctrler()->get_image();
    InfrastOperImageAnalyzer oper_analyzer(image);
    oper_analyzer.set_to_be_calced(InfrastOperImageAnalyzer::ToBeCalced::Mood);
    if (!oper_analyzer.analyze()) {
        Log.warn(__FUNCTION__, "No oper");
        return false;
    }
    oper_analyzer.sort_by_loc();

    const auto& ocr_replace = Task.get<OcrTaskInfo>("CharsNameOcrReplace");
    for (const auto& oper : oper_analyzer.get_result()) {
        RegionOCRer name_analyzer;
        name_analyzer.set_replace(ocr_replace->replace_map, ocr_replace->replace_full);
        name_analyzer.set_image(oper.name_img);
        name_analyzer.set_bin_expansion(0);
        if (!name_analyzer.analyze()) {
            continue;
        }
        std::string name = name_analyzer.get_result().text;
        if (oper.mood_ratio >= mood) {
            result.emplace_back(std::move(name));
        }
    }
    return true;
}

void asst::InfrastAbstractTask::order_opers_selection(const std::vector<std::string>& names)
{
    LogTraceFunction;

    if (names.empty()) {
        Log.warn("names is empty");
        return;
    }

    const auto image = ctrler()->get_image();
    InfrastOperImageAnalyzer oper_analyzer(image);
    oper_analyzer.set_to_be_calced(InfrastOperImageAnalyzer::ToBeCalced::Selected);
    if (!oper_analyzer.analyze()) {
        Log.warn("No oper");
        return;
    }
    oper_analyzer.sort_by_loc();
    const auto& ocr_replace = Task.get<OcrTaskInfo>("CharsNameOcrReplace");

    std::vector<TextRect> page_result;
    for (const auto& oper : oper_analyzer.get_result()) {
        RegionOCRer name_analyzer;
        name_analyzer.set_replace(ocr_replace->replace_map, ocr_replace->replace_full);
        name_analyzer.set_image(oper.name_img);
        name_analyzer.set_bin_expansion(0);
        if (!name_analyzer.analyze()) {
            continue;
        }
        TextRect tr = name_analyzer.get_result();
        tr.rect = oper.rect;
        page_result.emplace_back(std::move(tr));
    }

    for (const std::string& name : names) {
        auto iter = ranges::find_if(page_result, [&name](const TextRect& tr) { return tr.text == name; });
        if (iter != page_result.cend()) {
            ctrler()->click(iter->rect);
        }
        else {
            Log.error("name not in this page", name);
        }
    }
    sleep(500); // 此处刚刚选择了一位干员，因后续任务需截图识别，所以需要一个延迟，以保证后续截图选中状态无误
}

void asst::InfrastAbstractTask::click_return_button()
{
    LogTraceFunction;
    ProcessTask(*this, { "Infrast@ReturnButton" }).run();
}

bool asst::InfrastAbstractTask::click_bottom_left_tab()
{
    LogTraceFunction;

    ProcessTask task(*this, { "InfrastBottomLeftTab" });
    return task.run();
}

bool asst::InfrastAbstractTask::click_clear_button()
{
    LogTraceFunction;

    for (int i = 0; i != 5; ++i) {
        if (need_exit()) {
            return false;
        }
        bool ret = ProcessTask(*this, { "InfrastClearButton" }).run();
        // 有可能点快了，清空按钮刚刚出来，实际点上去还不生效，就点了
        // 所以多识别一次，如果没清掉就再清一下
        if (ret) {
            InfrastOperImageAnalyzer analyzer(ctrler()->get_image());
            analyzer.set_to_be_calced(InfrastOperImageAnalyzer::ToBeCalced::Selected);
            if (!analyzer.analyze()) {
                return false;
            }
            size_t selected_count =
                ranges::count_if(analyzer.get_result(), [](const infrast::Oper& info) { return info.selected; });
            Log.info(__FUNCTION__, "after clear, selected_count = ", selected_count);
            if (selected_count == 0) {
                break;
            }
        }
        else {
            Log.error(__FUNCTION__, "clear failed");
            return false;
        }
    }
    return true;
}

bool asst::InfrastAbstractTask::click_sort_by_trust_button()
{
    LogTraceFunction;

    ProcessTask task(*this, { "InfrastSortByTrustButton" });
    return task.run();
}

bool asst::InfrastAbstractTask::click_filter_menu_not_stationed_button()
{
    LogTraceFunction;

    ProcessTask task(*this, { "InfrastFilterMenuNotStationed" });
    return task.run();
}

bool asst::InfrastAbstractTask::click_filter_menu_cancel_not_stationed_button()
{
    LogTraceFunction;

    ProcessTask task(*this, { "InfrastFilterMenuCancelNotStationed" });
    return task.run();
}

bool asst::InfrastAbstractTask::click_confirm_button()
{
    LogTraceFunction;

    ProcessTask task(*this, { "InfrastDormConfirmButton" });
    return task.run();
}

void asst::InfrastAbstractTask::swipe_of_operlist()
{
    ProcessTask(*this, { "InfrastOperListSlowlySwipeToTheRight" }).run();
}

void asst::InfrastAbstractTask::swipe_to_the_left_of_operlist(int loop_times)
{
    if (loop_times < 0) {
        loop_times = operlist_swipe_times();
    }
    // 如果大于10，说明选择了排在很后面或者干脆不是这个站的技能的角色，例如加工站放红脸小车
    // 连续快速的滑动可能会丢失滑动距离，干脆多滑几次
    if (loop_times > 10) {
        loop_times = static_cast<int>(1.2 * loop_times) + 1;
    }
    for (int i = 0; i < loop_times; ++i) {
        ProcessTask(*this, { "InfrastOperListSwipeToTheLeft" }).run();
    }
    ProcessTask(*this, { "SleepAfterOperListQuickSwipe" }).run();
}

void asst::InfrastAbstractTask::swipe_to_the_left_of_main_ui()
{
    ProcessTask(*this, { "SwipeToTheLeft" }).run();
    sleep(500);
}

void asst::InfrastAbstractTask::swipe_to_the_right_of_main_ui()
{
    ProcessTask(*this, { "SwipeToTheRight" }).run();
    sleep(500);
}
