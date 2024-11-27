#include "BattleFormationTask.h"

#include "Config/GeneralConfig.h"
#include "Config/Miscellaneous/BattleDataConfig.h"
#include "Config/Miscellaneous/CopilotConfig.h"
#include "Config/Miscellaneous/SSSCopilotConfig.h"
#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "SupportList.h"
#include "Task/ProcessTask.h"
#include "Utils/ImageIo.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Ranges.hpp"
#include "Vision/MultiMatcher.h"

bool asst::BattleFormationTask::set_squad_index(const unsigned squad_index)
{
    LogTraceFunction;

    if (squad_index > NUM_SQUADS) {
        Log.error(__FUNCTION__, "| Invalid squad index:", squad_index);
        return true;
    }

    m_squad_index = squad_index;
    Log.info(__FUNCTION__, "| Formation will be performed on Squad", squad_index);
    return false;
}

bool asst::BattleFormationTask::set_specific_support_unit(const std::string& name)
{
    LogTraceFunction;

    if (m_support_unit_usage != SupportUnitUsage::Specific) {
        Log.error(__FUNCTION__, "| Current support unit usage is not SupportUnitUsage::Specific");
        return false;
    }

    m_specific_support_unit.name = name;
    // 之后在 parse_formation 中，如果发现这名助战干员，则将其技能设定为对应的所需技能
    m_specific_support_unit.skill = 0;
    return true;
};

bool asst::BattleFormationTask::_run()
{
    LogTraceFunction;

    if (!parse_formation()) {
        return false;
    }

    if (m_squad_index > 0 && !select_formation(m_squad_index)) {
        return false;
    }

    if (!enter_selection_page()) {
        save_img(utils::path("debug") / utils::path("other"));
        return false;
    }
    auto missing_operators = std::vector<OperGroup>();
    for (auto& [role, oper_groups] : m_formation) {
        add_formation(role, oper_groups, missing_operators);
    }

    // 在有且仅有一个缺失干员组时尝试寻找助战干员补齐编队
    if (m_support_unit_usage != SupportUnitUsage::None && missing_operators.size() == 1 &&
        m_used_support_unit_name.empty()) {
        // 先退出去招募助战再回来，好蠢
        confirm_selection();
        Log.info(__FUNCTION__, "| Left quick formation scene");

        // 如果指定助战干员正好可以补齐编队，则只招募指定助战干员就好了，记得再次确认一下 skill
        // 如果编队里正好有【艾雅法拉 - 2】和 【艾雅法拉 - 3】呢？—— 选第一个
        std::vector<battle::OperUsage>& required_opers = missing_operators.front().second;
        for (const battle::OperUsage& oper : required_opers) {
            if (oper.name == m_specific_support_unit.name) {
                m_specific_support_unit.skill = oper.skill;
                required_opers = { m_specific_support_unit };
                break;
            }
        }
        if (add_support_unit(required_opers, 5, true)) {
            missing_operators.clear();
        }

        // 再到快速编队页面
        if (!enter_selection_page()) {
            save_img(utils::path("debug") / utils::path("other"));
            return false;
        }
        Log.info(__FUNCTION__, "| Returned to quick formation scene");
    }

    // 在尝试补齐编队后依然有缺失干员，自动编队失败
    if (!missing_operators.empty()) {
        report_missing_operators(missing_operators);
        return false;
    }

    // 对于有在干员组中存在的自定干员，无法提前得知是否成功编入，故不提前加入编队
    if (!m_user_additional.empty()) {
        auto limit = 12 - m_size_of_operators_in_formation;
        for (const auto& [name, skill] : m_user_additional) {
            if (m_opers_in_formation->contains(name)) {
                continue;
            }
            if (--limit < 0) {
                break;
            }
            asst::battle::OperUsage oper;
            oper.name = name;
            oper.skill = skill;
            std::vector<asst::battle::OperUsage> usage { std::move(oper) };
            m_user_formation[BattleData.get_role(name)].emplace_back(name, std::move(usage));
        }
        for (auto& [role, oper_groups] : m_user_formation) {
            add_formation(role, oper_groups, missing_operators);
        }
    }

    add_supplementary_opers();
    // if (m_add_trust) {
    //     add_trust_operators();
    // }
    confirm_selection();

    if (m_used_support_unit_name.empty()) {
        if (m_support_unit_usage == SupportUnitUsage::Specific) { // 使用指定助战干员
            add_support_unit({ m_specific_support_unit }, 5, true);
        }
        else if (m_support_unit_usage == SupportUnitUsage::Random) { // 使用随机助战干员
            add_support_unit({}, 5, false);
        }
    }

    return true;
}

bool asst::BattleFormationTask::add_formation(
    battle::Role role,
    std::vector<OperGroup> oper_group,
    std::vector<OperGroup>& missing)
{
    LogTraceFunction;

    // 初始化变量
    OperList oper_list(m_inst, *this);

    oper_list.select_role(role);
    m_last_oper_name.clear();
    bool has_error = false;
    int swipe_times = 0;
    int overall_swipe_times = 0; // 完整从左到右滑动的次数
    while (!need_exit()) {
        if (select_opers_in_cur_page(oper_group)) {
            has_error = false;
            if (oper_group.empty()) {
                break;
            }
            swipe_page();
            ++swipe_times;
        }
        else if (has_error) {
            swipe_to_the_left(swipe_times);
            // reset page
            oper_list.select_role(role, true);
            m_last_oper_name.clear();
            swipe_to_the_left(swipe_times);
            swipe_times = 0;
            has_error = false;
        }
        else {
            if (overall_swipe_times == m_missing_retry_times) {
                missing.insert(missing.end(), oper_group.begin(), oper_group.end());
                return true;
            }

            ++overall_swipe_times;

            has_error = true;
            swipe_to_the_left(swipe_times);
            swipe_times = 0;
        }
    }
    return true;
}

bool asst::BattleFormationTask::select_random_support_unit()
{
    return ProcessTask(*this, { "BattleSupportUnitFormation" }).run();
}

void asst::BattleFormationTask::report_missing_operators(std::vector<OperGroup>& groups)
{
    auto info = basic_info();

    std::vector<std::vector<std::string>> oper_names;
    for (auto& group : groups) {
        std::vector<std::string> names;
        for (const auto& oper : group.second) {
            names.push_back(oper.name);
        }
        oper_names.push_back(names);
    }

    info["why"] = "OperatorMissing";

    info["details"] = json::object { { "opers", json::array(oper_names) } };
    callback(AsstMsg::SubTaskError, info);
}

std::vector<asst::TemplDetOCRer::Result> asst::BattleFormationTask::analyzer_opers()
{
    auto formation_task_ptr = Task.get("BattleQuickFormationOCR");
    auto image = ctrler()->get_image();
    const auto& ocr_replace = Task.get<OcrTaskInfo>("CharsNameOcrReplace");
    std::vector<TemplDetOCRer::Result> opers_result;

    for (int i = 0; i < 8; ++i) {
        std::string task_name = "BattleQuickFormation-OperNameFlag" + std::to_string(i);

        const auto& params = Task.get("BattleQuickFormationOCR")->special_params;
        TemplDetOCRer name_analyzer(image);

        name_analyzer.set_task_info(task_name, "BattleQuickFormationOCR");
        name_analyzer.set_bin_threshold(params[0]);
        name_analyzer.set_bin_expansion(params[1]);
        name_analyzer.set_bin_trim_threshold(params[2], params[3]);
        name_analyzer.set_replace(ocr_replace->replace_map, ocr_replace->replace_full);
        auto cur_opt = name_analyzer.analyze();
        if (!cur_opt) {
            continue;
        }
        for (auto& res : *cur_opt) {
            constexpr int kMinDistance = 5;
            auto find_it = ranges::find_if(opers_result, [&res](const TemplDetOCRer::Result& pre) {
                return std::abs(pre.flag_rect.x - res.flag_rect.x) < kMinDistance &&
                       std::abs(pre.flag_rect.y - res.flag_rect.y) < kMinDistance;
            });
            if (find_it != opers_result.end() || res.text.empty()) {
                continue;
            }
            opers_result.emplace_back(std::move(res));
        }
    }

    if (opers_result.empty()) {
        Log.error("BattleFormationTask: no oper found");
        return {};
    }
    sort_by_vertical_(opers_result);

    Log.info(opers_result);
    return opers_result;
}

bool asst::BattleFormationTask::enter_selection_page()
{
    return ProcessTask(*this, { "BattleQuickFormation" }).set_retry_times(3).run();
}

bool asst::BattleFormationTask::select_opers_in_cur_page(std::vector<OperGroup>& groups)
{
    auto opers_result = analyzer_opers();

    static const std::array<Rect, 3> SkillRectArray = {
        Task.get("BattleQuickFormationSkill1")->specific_rect,
        Task.get("BattleQuickFormationSkill2")->specific_rect,
        Task.get("BattleQuickFormationSkill3")->specific_rect,
    };

    if (!opers_result.empty()) {
        if (m_last_oper_name == opers_result.back().text) {
            Log.info("last oper name is same as current, skip");
            return false;
        }
        m_last_oper_name = opers_result.back().text;
    }

    int delay = Task.get("BattleQuickFormationOCR")->post_delay;
    int skill = 0;
    for (const auto& res : opers_result) {
        const std::string& name = res.text;
        bool found = false;
        auto iter = groups.begin();
        for (; iter != groups.end(); ++iter) {
            for (const auto& oper : iter->second) {
                if (oper.name == name) {
                    found = true;
                    skill = oper.skill;

                    m_opers_in_formation->emplace(name, iter->first);
                    ++m_size_of_operators_in_formation;
                    break;
                }
            }
            if (found) {
                break;
            }
        }

        if (iter == groups.end()) {
            continue;
        }

        ctrler()->click(res.flag_rect);
        sleep(delay);
        if (1 <= skill && skill <= 3) {
            if (skill == 3) {
                ProcessTask(*this, { "BattleQuickFormationSkill-SwipeToTheDown" }).run();
            }
            ctrler()->click(SkillRectArray.at(skill - 1ULL));
            sleep(delay);
        }
        groups.erase(iter);

        json::value info = basic_info_with_what("BattleFormationSelected");
        auto& details = info["details"];
        details["selected"] = name;
        callback(AsstMsg::SubTaskExtraInfo, info);
    }

    return true;
}

void asst::BattleFormationTask::swipe_page()
{
    ProcessTask(*this, { "OperList-SwipeToTheRight" }).run();
}

void asst::BattleFormationTask::swipe_to_the_left(int times)
{
    for (int i = 0; i < times; ++i) {
        ProcessTask(*this, { "BattleFormationOperListSwipeToTheLeft" }).run();
    }
    sleep(Config.get_options().task_delay); // 可能有界面回弹，睡一会儿
}

bool asst::BattleFormationTask::confirm_selection()
{
    return ProcessTask(*this, { "BattleQuickFormationConfirm" }).run();
}

bool asst::BattleFormationTask::parse_formation()
{
    json::value info = basic_info_with_what("BattleFormation");
    auto& details = info["details"];
    auto& formation = details["formation"];

    const RequiredOperGroups& required_oper_groups =
        m_data_resource == DataResource::SSSCopilot ? SSSCopilot.get_data().groups : Copilot.get_data().groups;

    for (const auto& [group_name, required_opers] : required_oper_groups) {
        if (required_opers.empty()) {
            continue;
        }
        formation.emplace(group_name);

        // 判断干员/干员组的职业，放进对应的分组
        bool same_role = true;
        battle::Role role = BattleData.get_role(required_opers.front().name);
        for (const auto& required_oper : required_opers) {
            same_role &= BattleData.get_role(required_oper.name) == role;

            // （仅一次）如果发现这名助战干员，则将其技能设定为对应的所需技能
            if (required_oper.name == m_specific_support_unit.name && m_specific_support_unit.skill == 0) {
                m_specific_support_unit.skill = required_oper.skill;
            }
        }

        // for unknown, will use { "All@OperList-SelectRole", "All@OperList-SelectRole-OCR" }
        m_formation[same_role ? role : battle::Role::Unknown].emplace_back(group_name, required_opers);
    }

    callback(AsstMsg::SubTaskExtraInfo, info);
    return true;
}

bool asst::BattleFormationTask::select_formation(int select_index)
{
    // 编队不会触发改名的区域有两组
    // 一组是上面的黑长条 260*9
    // 第二组是名字最左边和最右边的一块区域
    // 右边比左边窄，暂定为左边 10*58

    static const std::vector<std::string> select_formation_task = { "BattleSelectFormation1",
                                                                    "BattleSelectFormation2",
                                                                    "BattleSelectFormation3",
                                                                    "BattleSelectFormation4" };

    return ProcessTask { *this, { select_formation_task[select_index - 1] } }.run();
}

// ————————————————————————————————————————————————————————————————————————————————
// Supplementary Operator-Related
// ————————————————————————————————————————————————————————————————————————————————
void asst::BattleFormationTask::add_supplementary_opers()
{
    LogTraceFunction;

    using battle::CandidateOper;

    // 初始化变量
    OperList oper_list(m_inst, *this);

    // 取消对特殊关注的置顶
    if (!oper_list.unpin_marked_opers()) {
        Log.error(__FUNCTION__, "| Failed to unpin marked opers; abandoning the addition of supplementary operators.");
        return;
    }

    for (const auto& [role, sort_key, ascending, num] : m_supplementary_oper_reqs) {
        Log.info(
            __FUNCTION__,
            "| Trying add",
            num,
            "supplementary operator(s) of role",
            enum_to_string(role),
            "after sorting operator list by",
            enum_to_string(sort_key),
            "in",
            ascending ? "ascending" : "descending",
            "order");

        oper_list.select_role(role, true);   // 选择职业并重置列表位置
        oper_list.sort(sort_key, ascending); // 重新排序

        unsigned added_num = 0;              // 用于记录此轮增加的干员数量

        while (added_num < num && !need_exit()) {
            const std::optional<unsigned> opt = oper_list.update_page();
            if (!opt.has_value()) { // 出错
                Log.info(__FUNCTION__, "| Error detected when analyzing current operator list page.");
                break;
            }
            if (opt.value() == 0) { // 无新增干员/已经到达最后一页
                Log.info(__FUNCTION__, "| No new operators found. End of operator list reached.");
                break;
            }

            // 新增干员 index 范围为 [oper_list.size() - opt.value(), oper_list.size() - 1]
            for (size_t index = oper_list.size() - opt.value(); index < oper_list.size(); ++index) {
                const CandidateOper& candidate_oper = oper_list[index];

                if (!candidate_oper.selected) {
                    if (oper_list.select_oper(index)) {
                        ++added_num;
                    }
                    else {
                        // 编队已满，跳出循环
                        Log.info(
                            __FUNCTION__,
                            "Formation is now full; abandoning the addition of supplementary operators.");
                        m_formation_is_full = true;
                        break;
                    }
                }

                if (added_num < num) {
                    oper_list.move_forward();
                }
            }                          // inner for loop to traverse indices of newly-detected operators

            if (m_formation_is_full) { // 编队已满，跳出循环
                break;
            }
        } // while (added_num < num && !need_exit())

        Log.info(__FUNCTION__, "| Added", added_num, "operator(s).");

        if (m_formation_is_full) { // 编队已满，跳出循环
            break;
        }
    } // outer for loop to traverse m_supplementary_oper_reqs
}

// ————————————————————————————————————————————————————————————————————————————————
// Support Unit-Related
// ————————————————————————————————————————————————————————————————————————————————
bool asst::BattleFormationTask::add_support_unit(
    const std::vector<RequiredOper>& required_opers,
    const int max_refresh_times,
    const bool max_spec_lvl,
    const bool allow_non_friend_support_unit)
{
    LogTraceFunction;

    // 通过点击编队界面右上角 <助战单位> 文字左边的 Icon 进入助战干员选择界面
    ProcessTask(*this, { "UseSupportUnit-EnterSupportList" }).run();

    // 随机模式
    if (required_opers.empty()) {
        if (add_support_unit_(required_opers, max_refresh_times, max_spec_lvl, allow_non_friend_support_unit)) {
            return true;
        }
    }
    else {
        // 非随机模式
        std::vector<RequiredOper> temp_required_opers;
        for (size_t i = 0; i < 3; ++i) {
            if (i >= required_opers.size()) {
                break;
            }
            temp_required_opers.emplace_back(required_opers[i]);
            if (add_support_unit_(
                    temp_required_opers,
                    max_refresh_times,
                    max_spec_lvl,
                    allow_non_friend_support_unit)) {
                return true;
            }
        }
    }

    // 未找到符合要求的助战干员，手动退出助战列表
    Log.info(__FUNCTION__, "| Fail to find any qualified support operator.");
    ProcessTask(*this, { "UseSupportUnit-LeaveSupportList" }).run();
    return false;
}

bool asst::BattleFormationTask::add_support_unit_(
    const std::vector<RequiredOper>& required_opers,
    const int max_refresh_times,
    const bool max_spec_lvl,
    const bool allow_non_friend_support_unit)
{
    LogTraceFunction;

    using battle::SupportUnit;

    // 初始化变量
    SupportList support_list(m_inst, *this);
    std::vector<std::optional<size_t>> candidates(required_opers.size(), std::nullopt);
    const bool random_mode = required_opers.empty();

    // 随机模式下保留当前职业选择，否则切换到 required_opers.back() 对应职业的助战干员列表
    if (!random_mode && !support_list.select_role(BattleData.get_role(required_opers.back().name))) {
        Log.error(__FUNCTION__, "Failed to select role; abandoning using support unit.");
        return false;
    }

    for (int refresh_times = 0; refresh_times <= max_refresh_times && !need_exit(); ++refresh_times) {
        // Step 1: 获取助战干员列表
        support_list.update();

        // Step 2: 遍历助战栏位，筛选助战干员
        for (size_t index = 0; index < support_list.size(); ++index) {
            const SupportUnit& support_unit = support_list[index];

            // 若 support_unit 满足以下筛选条件：
            // 1. 当 max_spec_lvl == true 时，若稀有度大于等于 4 星则精英化等级达到 2，
            // 若稀有度为 3 星则精英化等级达到 1；
            // 2. 当 allow_non_friend_support_unit == false 时，必须满足 support_unit.from_friend == true；
            // 3. 在非随机模式下存在 required_opers[i] 与之匹配：
            // 3a. required_opers[i].name == name;
            // 3b. support_unit.elite >= required_opers[i].skill - 1;
            // 则将 candidates[i] 设置为 index。
            if (max_spec_lvl && ((BattleData.get_rarity(support_unit.name) >= 4 && support_unit.elite < 2) ||
                                 (BattleData.get_rarity(support_unit.name) == 3 && support_unit.elite < 1))) {
                continue;
            }
            if (!allow_non_friend_support_unit && !support_unit.from_friend) {
                continue;
            }
            // 随机模式下直接选择这名干员，使用其默认技能
            if (random_mode) {
                if (support_list.use_support_unit(index, 0)) {
                    m_used_support_unit_name = support_list[index].name;
                    // callback
                    json::value info = basic_info_with_what("RecruitSuppportOperator");
                    info["details"]["name"] = support_list[index].name;
                    callback(AsstMsg::SubTaskExtraInfo, info);
                    return true;
                }
                return false;
            }
            // 非随机模式下
            for (size_t i = 0; i < required_opers.size(); ++i) {
                const RequiredOper& required_oper = required_opers[i];
                if (support_unit.name == required_oper.name && support_unit.elite >= required_oper.skill - 1) {
                    candidates[i] = index;
                }
            }
        }

        // Step 3: 依次点选筛选出的助战干员，根据需要判断技能是否为专三，并使用
        for (size_t i = 0; i < required_opers.size(); ++i) {
            if (!candidates[i].has_value()) {
                continue;
            }
            if (support_list.use_support_unit(candidates[i].value(), required_opers[i].skill, max_spec_lvl)) {
                m_used_support_unit_name = support_list[candidates[i].value()].name;
                // callback
                json::value info = basic_info_with_what("RecruitSuppportOperator");
                info["details"]["name"] = support_list[candidates[i].value()].name;
                callback(AsstMsg::SubTaskExtraInfo, info);
                return true;
            }
        }

        // 重置变量
        candidates.assign(required_opers.size(), std::nullopt);

        // 更新助战列表
        if (refresh_times < max_refresh_times) {
            support_list.refresh_list();
        }
    } // outer for loop to iterate until reaching max_refresh_times

    Log.info(__FUNCTION__, "| Fail to find any qualified support operator.");
    return false;
}
