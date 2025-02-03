#include "RoguelikeRecruitTaskPlugin.h"

#include "Config/Miscellaneous/BattleDataConfig.h"
#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Matcher.h"
#include "Vision/OCRer.h"
#include "Vision/Roguelike/RoguelikeRecruitImageAnalyzer.h"
#include "Vision/Roguelike/RoguelikeRecruitSupportAnalyzer.h"

using namespace asst::battle::roguelike;

bool asst::RoguelikeRecruitTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskCompleted || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    if (!RoguelikeConfig::is_valid_theme(m_config->get_theme())) {
        Log.error("Roguelike name doesn't exist!");
        return false;
    }
    const std::string roguelike_name = m_config->get_theme() + "@";
    const std::string& task = details.get("details", "task", "");
    std::string_view task_view = task;
    if (task_view.starts_with(roguelike_name)) {
        task_view.remove_prefix(roguelike_name.length());
    }
    if (task_view == "Roguelike@ChooseOper") {
        return true;
    }
    else {
        return false;
    }
}

asst::battle::Role asst::RoguelikeRecruitTaskPlugin::get_oper_role(const std::string& name)
{
    return BattleData.get_role(name);
}

bool asst::RoguelikeRecruitTaskPlugin::is_oper_melee(const std::string& name)
{
    const auto role = get_oper_role(name);
    if (role != battle::Role::Pioneer && role != battle::Role::Tank && role != battle::Role::Warrior) {
        return false;
    }
    const auto loc = BattleData.get_location_type(name);
    return loc == battle::LocationType::Melee;
}

std::unordered_set<std::string> asst::RoguelikeRecruitTaskPlugin::calculate_condition_oper(
    const RecruitPriorityOffset& condition,
    const std::unordered_map<std::string, RoguelikeOper>& chars_map)
{
    std::unordered_set<std::string> opers; // 符合这个策略组的干员
    for (const auto& [oper_name, oper_level] : chars_map) {
        if (condition.opers.contains(oper_name)) {
            opers.insert(oper_name);
        }
    }
    return opers;
}

bool asst::RoguelikeRecruitTaskPlugin::_run()
{
    LogTraceFunction;

    ++m_recruit_count;

    if (m_config->get_theme() == "Sarkaz" && m_config->get_mode() == RoguelikeMode::Investment &&
        (m_config->get_squad() == "点刺成锭分队" ||
         (m_config->get_squad() == "后勤分队" && m_config->get_start_with_seed()))) {
        ProcessTask(*this, { "Sarkaz@RoguelikeRecruit-GiveUp" }).run();
        return true;
    }

    if (m_config->get_mode() == RoguelikeMode::Investment && m_recruit_count > 1 &&
        m_config->get_squad() == "蓝图测绘分队") {
        // 如果是投资模式，直接招募第一个干员
        lazy_recruit();
        return true;
    }

    // 是否使用助战干员开局
    if (m_config->get_use_support()) {
        if (recruit_support_char()) {
            m_starts_complete = true;
            return true;
        }
    }
    else {
        if (recruit_own_char()) {
            m_starts_complete = true;
            return true;
        }
    }

    bool team_full_without_rookie = m_config->get_team_full_without_rookie();
    // Log.info("team_full_without_rookie", team_full_without_rookie);

    // 编队信息 (已有角色)
    const auto& chars_map = m_config->status().opers;

    // __________________will-be-removed-begin__________________
    std::unordered_map<battle::Role, int> team_roles;
    int offset_melee_num = 0;
    for (const auto& [name, oper] : chars_map) {
        if (name.starts_with("预备干员")) {
            continue;
        }
        team_roles[battle::get_role_type(name)]++;
        if (is_oper_melee(name)) {
            offset_melee_num++;
        }
    }
    // __________________will-be-removed-end__________________

    if (!m_starts_complete) {
        for (const auto& oper : chars_map) {
            auto& recruit_info = RoguelikeRecruit.get_oper_info(m_config->get_theme(), oper.first);
            if (recruit_info.is_start) {
                m_starts_complete = true;
            }
        }
    }

    if (!m_team_complete) {
        bool complete = true;
        int complete_count = 0;
        const auto& team_complete_condition = RoguelikeRecruit.get_team_complete_info(m_config->get_theme());
        for (const auto& condition : team_complete_condition) { // 每个完备度的策略组
            std::unordered_set<std::string> opers =
                calculate_condition_oper(condition, chars_map); // 符合这个策略组的干员

            complete_count += int(opers.size());
            if (int(opers.size()) < condition.threshold) {
                complete = false;
            }

            Log.trace("__FUNCTION__", "groups:", condition.groups);
            Log.trace("__FUNCTION__", "opers:", opers);
        }
        m_team_complete = complete;
        if (complete_count <= RoguelikeRecruit.get_team_complete_require(m_config->get_theme()) / 2 &&
            m_recruit_count >= 10) {
            // 如果第10次招募还没拿到半队key干员，说明账号阵容不齐，放开招募限制，有啥用啥吧
            m_team_complete = true;
        }
        Log.trace("__FUNCTION__", "complete_count:", complete_count, "m_team_complete:", m_team_complete);
    }

    if (m_recruit_count >= 3 && !m_starts_complete) {
        // 如果第3次招募还没拿到start干员，说明账号练度低且阵容不齐，放开招募限制，有啥用啥吧
        m_starts_complete = true;
        m_team_complete = true;
    }

    // 候选干员
    std::vector<RoguelikeRecruitInfo> recruit_list;

    // 干员名字的识别位置
    std::unordered_map<std::string, Rect> last_oper_rects;

    int SwipeTimes = Task.get("RoguelikeRecruitSwipeMaxTime")->max_times;
    int i = 0;
    std::unordered_set<std::string> pre_oper_names;

    // 开始识别前先往前翻两页，是方舟的bug，有可能进招募界面时不是从最左边开始
    swipe_to_the_left_of_operlist();

    bool temp_recruit_exist = false; // 检测是否存在临时招募
    auto image = ctrler()->get_image();
    Matcher temp_recruit_analyzer(image);
    temp_recruit_analyzer.set_task_info("Roguelike@TempRecruitFlag");
    temp_recruit_exist = temp_recruit_analyzer.analyze().has_value();
    Log.trace(__FUNCTION__, "temp_recruit_exist", temp_recruit_exist);

    // 翻页找出所有候选干员
    for (; i != SwipeTimes; ++i) {
        if (need_exit()) {
            return false;
        }
        image = ctrler()->get_image();
        RoguelikeRecruitImageAnalyzer analyzer(image);
        if (!analyzer.analyze()) {
            Log.trace(__FUNCTION__, "| Page", i, "recruit list analyse failed");
            // 还没滑动就识别失败，通常是招募界面为空，视为招募成功并退出
            if (i == 0) {
                return true;
            }
            // 已经滑动过，识别失败可能是干员不可选
            break;
        }

        std::unordered_set<std::string> oper_names;
        const auto& oper_list = analyzer.get_result();
        bool stop_swipe = false;
        const auto& owned_collection = m_config->status().collections;

        int max_oper_x = 0;
        for (const auto& oper_info : oper_list) {
            oper_names.emplace(oper_info.name);

            max_oper_x = std::max(max_oper_x, oper_info.rect.x);
            // 查询上次识别位置
            const auto& rect_it = last_oper_rects.find(oper_info.name);
            if (rect_it == last_oper_rects.cend()) {
                last_oper_rects.emplace(oper_info.name, oper_info.rect);
            }
            else {
                const auto x_offset = abs(rect_it->second.x - oper_info.rect.x);
                const auto y_offset = abs(rect_it->second.y - oper_info.rect.y);

                // 偏移低于阈值，代表划动没有效果，已经到达屏幕最右侧
                if (x_offset < 20 && y_offset < 5) {
                    stop_swipe = true;
                    Log.trace(
                        __FUNCTION__,
                        "| Page",
                        i,
                        "oper",
                        oper_info.name,
                        "last rect:",
                        rect_it->second.to_string(),
                        "current rect:",
                        oper_info.rect.to_string(),
                        " - stop swiping");
                    break;
                }

                rect_it->second = oper_info.rect;
            }

            // 查询招募配置
            const auto& recruit_info = RoguelikeRecruit.get_oper_info(m_config->get_theme(), oper_info.name);
            int priority = 0;

            // 查询编队是否已持有该干员
            if (const auto& char_opt = chars_map.find(oper_info.name); char_opt != chars_map.end()) {
                // 干员已在编队中，又出现在招募列表，只有待晋升和预备干员两种情况
                if (recruit_info.is_alternate) {
                    // 预备干员可以重复招募，但是最好不要重复招募预备干员占用编队位置
                    priority = recruit_info.recruit_priority_when_team_full;
                }
                else {
                    // 干员待晋升
                    priority = team_full_without_rookie ? recruit_info.promote_priority_when_team_full
                                                        : recruit_info.promote_priority;
                    if (temp_recruit_exist) { // 临时招募干员的进阶优先级略微提高
                        priority += 150;
                        temp_recruit_exist = false;
                    }
                    if (oper_info.elite < 2) { // 若未精二，则晋升无意义
                        priority = 0;
                    }
                }
            }
            else {
                // 干员待招募，检查练度
                if (oper_info.elite == 2) {
                    // TODO: 招募时已精二 (随机提升或对应分队) => promoted_priority
                    priority = team_full_without_rookie ? recruit_info.recruit_priority_when_team_full +
                                                              recruit_info.promote_priority_when_team_full
                                                        : recruit_info.recruit_priority + recruit_info.promote_priority;
                }
                else if (oper_info.elite == 1 && (oper_info.level >= 55 || oper_info.level == 0)) {
                    // 精一55级以上
                    // 等级是 0 一般是识别错了，多发生于外服，一般都是一错全错，先凑合着招一个吧，比不招人强 orz
                    priority = team_full_without_rookie ? recruit_info.recruit_priority_when_team_full
                                                        : recruit_info.recruit_priority;
                }
                else {
                    // 精一55级以下，默认不招募
                    Log.trace(
                        __FUNCTION__,
                        "| Ignored low level oper:",
                        oper_info.name,
                        oper_info.elite,
                        oper_info.level);
                }

                if (temp_recruit_exist && !oper_info.name.starts_with("预备干员")) {
                    // 临时招募干员具有极高抓取优先级，但是在编队时会占用前面的位置
                    priority += 600;
                    temp_recruit_exist = false;
                }

                if (!recruit_info.is_alternate) {
                    // __________________will-be-removed-begin__________________
                    const battle::Role oper_role = get_oper_role(oper_info.name);
                    int role_num = recruit_info.offset_melee ? offset_melee_num : team_roles[oper_role];
                    for (const auto& offset_pair : ranges::reverse_view(recruit_info.recruit_priority_offset)) {
                        if (role_num >= offset_pair.first) {
                            priority += offset_pair.second;
                            break;
                        }
                    }
                    // role_num = team_roles[oper_role];
                    // const auto role_info = RoguelikeRecruit.get_role_info(rogue_theme, oper_role);
                    // for (const auto& offset_pair : ranges::reverse_view(role_info)) {
                    //     if (role_num >= offset_pair.first) {
                    //         priority += offset_pair.second;
                    //         break;
                    //     }
                    // }
                    //  __________________will-be-removed-end__________________
                    for (const auto& priority_offset : recruit_info.recruit_priority_offsets) {
                        std::unordered_set<std::string> opers = // 符合这个策略组的干员
                            calculate_condition_oper(priority_offset, chars_map);
                        if (priority_offset.is_less) {
                            if (int(opers.size()) <= priority_offset.threshold) {
                                priority += priority_offset.offset;
                            }
                        }
                        else {
                            if (int(opers.size()) >= priority_offset.threshold) {
                                priority += priority_offset.offset;
                            }
                        }
                    }
                    for (const auto& priority_offset : recruit_info.collection_priority_offsets) {
                        auto iter =
                            std::find(owned_collection.begin(), owned_collection.end(), priority_offset.collection);
                        if (iter != owned_collection.end()) {
                            priority += priority_offset.offset;
                        }
                    }
                    if (!m_starts_complete) {
                        if (!recruit_info.is_start) {
                            priority -= 1000;
                        }
                    }
                    else if (!m_team_complete) {
                        if (!recruit_info.is_key) {
                            priority -= 1000;
                        }
                    }
                }
            }

            // 优先级为0，可能练度不够被忽略
            if (priority <= 0) {
                continue;
            }

            // 添加到候选名单
            auto existing_it = ranges::find_if(recruit_list, [&](const RoguelikeRecruitInfo& pri) -> bool {
                return pri.name == recruit_info.name;
            });
            if (existing_it == recruit_list.cend()) {
                RoguelikeRecruitInfo info;
                info.name = recruit_info.name;
                info.priority = priority;
                info.is_alternate = recruit_info.is_alternate;
                info.page_index = i;
                recruit_list.emplace_back(info);
            }
            Log.info(__FUNCTION__, "| Operator", recruit_info.name, "priority:", priority);
        }

        Log.info(__FUNCTION__, "| Page", i, "oper list:", oper_names);

        if (stop_swipe) {
            break;
        }

        // 每列4个干员，未滑动时可以显示2列，滑动后至少可以显示1列
        const size_t oper_count = oper_list.size();
        if ((i == 0 && oper_count < 8) || (oper_count != 4 && oper_count != 8)) {
            Log.trace(__FUNCTION__, "| Page", i, "oper count:", oper_count, "- stop swiping");
            break;
        }
        if (pre_oper_names == oper_names) {
            Log.trace(__FUNCTION__, "| Oper list not changed, stop swiping");
            break;
        }
        pre_oper_names = std::move(oper_names);

        // 向右滑动
        Log.trace(__FUNCTION__, "| Page", i, "oper count:", oper_count, "- continue swiping");
        slowly_swipe(false, max_oper_x - 200);
        sleep(Task.get("RoguelikeCustom-HijackCoChar")->post_delay);
    }

    bool recruited = false;
    auto recruit_oper = [&](const battle::roguelike::Recruitment& info) {
        select_oper(info);
        recruited = true;
    };

    // 没有候选干员，进入后备逻辑
    if (recruit_list.empty()) {
        Log.trace(__FUNCTION__, "| No oper in recruit list.");

        // 如果划动过，先划回最左边
        if (i != 0) {
            swipe_to_the_left_of_operlist(i + 1);
        }

        image = ctrler()->get_image();
        RoguelikeRecruitImageAnalyzer analyzer(image);
        if (!analyzer.analyze()) {
            Log.error(__FUNCTION__, "| Random recruitment analyse failed");
            return false;
        }

        const auto& oper_list = analyzer.get_result();

        // 可能有临时招募的精二的，不要白不要
        for (const auto& info : oper_list) {
            if (info.elite != 2) {
                continue;
            }
            Log.trace(__FUNCTION__, "| Choose temporary recruitment elite 2:", info.name, info.elite, info.level);
            recruit_oper(info);
            return true;
        }

        //// 仍然没选，随便选个精一 50 以上的
        // for (const auto& info : oper_list) {
        //     if (info.elite == 0 || (info.elite == 1 && info.level < 50)) {
        //         continue;
        //     }
        //     Log.trace(__FUNCTION__, "| Choose random elite 1:", info.name, info.elite, info.level);
        //     recruit_oper(info);
        //     return true;
        // }

        Log.trace(__FUNCTION__, "| Did not choose oper");
        return true;
    }

    // 选择优先级最高的干员
    auto selected_oper = ranges::max_element(recruit_list, std::less {}, std::mem_fn(&RoguelikeRecruitInfo::priority));
    if (selected_oper == recruit_list.cend()) {
        Log.trace(__FUNCTION__, "| No opers in recruit list.");
        return false;
    }

    std::string char_name = selected_oper->name;
    Log.trace(
        __FUNCTION__,
        "| Top priority oper:",
        char_name,
        selected_oper->priority,
        "page",
        selected_oper->page_index,
        "/",
        i);

    // 滑动方向
    // 页码大于一半: 从右往左划动
    // 页码小于一半: 划回最左边再往右
    bool is_rtl = false;

    // 如果划动过，判断目标角色离哪个方向更近
    if (i != 0) {
        is_rtl = (selected_oper->page_index * 2) >= i;
        if (!is_rtl) {
            // 从左往右需要先划回最左边
            swipe_to_the_left_of_operlist(i + 1);
        }
    }

    return recruit_appointed_char(char_name, is_rtl);
}

void asst::RoguelikeRecruitTaskPlugin::reset_in_run_variables()
{
    m_recruit_count = 0;
    m_starts_complete = false;
    m_team_complete = false;
}

bool asst::RoguelikeRecruitTaskPlugin::lazy_recruit()
{
    LogTraceFunction;

    ProcessTask(*this, { "RoguelikeRecruitLazyClick1" }).run();
    return true;
}

bool asst::RoguelikeRecruitTaskPlugin::recruit_appointed_char(const std::string& char_name, bool is_rtl)
{
    LogTraceFunction;
    // 最大滑动次数
    int SwipeTimes = Task.get("RoguelikeRecruitSwipeMaxTime")->max_times;
    std::unordered_set<std::string> pre_oper_names;
    bool has_been_same = false;
    int i = 0;
    // 是否凹直升
    bool start_with_elite_two = m_config->get_start_with_elite_two();
    bool only_start_with_elite_two = m_config->get_only_start_with_elite_two();

    for (; i != SwipeTimes; ++i) {
        if (need_exit()) {
            return false;
        }
        auto image = ctrler()->get_image();
        RoguelikeRecruitImageAnalyzer analyzer(image);

        int max_oper_x = 700;
        // 只处理识别成功的情况，失败(无任何结果)时继续滑动
        if (analyzer.analyze()) {
            const auto& chars = analyzer.get_result();
            max_oper_x = ranges::max(chars | views::transform([&](const auto& x) { return x.rect.x; }));
            auto it = ranges::find_if(chars, [&](const battle::roguelike::Recruitment& oper) -> bool {
                return oper.name == char_name;
            });

            std::unordered_set<std::string> oper_names;
            ranges::transform(
                chars,
                std::inserter(oper_names, oper_names.end()),
                std::mem_fn(&battle::roguelike::Recruitment::name));
            Log.info(__FUNCTION__, "| Oper list:", oper_names);

            if (it != chars.cend()) {
                // !get_run_for_collectible() 即当前没有在烧开水/水已经烧好了
                // 需要凹直升且，要么已经烧好了水，要么只需要凹直升不需要烧水
                if (start_with_elite_two && (!m_config->get_run_for_collectible() || only_start_with_elite_two)) {
                    if (it->elite == 2) {
                        m_task_ptr->set_enable(false);
                    }
                    else {
                        // 非只凹直升时，重新烧水
                        if (!only_start_with_elite_two) {
                            m_config->set_run_for_collectible(true);
                        }
                        m_control_ptr->exit_then_stop();
                    }
                }
                else {
                    select_oper(*it);
                }
                return true;
            }
            if (pre_oper_names == oper_names) {
                if (has_been_same) {
                    Log.trace(__FUNCTION__, "| Oper list not changed for three times, stop swiping");
                    break;
                }
                else {
                    has_been_same = true;
                }
            }
            else {
                has_been_same = false;
                pre_oper_names = std::move(oper_names);
            }
        }

        // 没识别到目标干员，可能不在这一页，继续划动
        if (is_rtl) {
            slowly_swipe(true);
        }
        else {
            slowly_swipe(false, max_oper_x - 200);
        }
        sleep(Task.get("RoguelikeCustom-HijackCoChar")->post_delay);
    }
    Log.info(__FUNCTION__, "| Cannot find oper `" + char_name + "`");
    swipe_to_the_left_of_operlist(i + 1);
    return false;
}

bool asst::RoguelikeRecruitTaskPlugin::recruit_support_char()
{
    LogTraceFunction;
    const int MaxRefreshTimes = Task.get("RoguelikeRefreshSupportBtnOcr")->special_params.front();

    auto core_opt = m_config->get_core_char();
    m_config->set_core_char("");
    if (!core_opt.empty()) {
        if (recruit_support_char(core_opt, MaxRefreshTimes)) {
            return true;
        }
    }
    return false;
}

bool asst::RoguelikeRecruitTaskPlugin::recruit_support_char(const std::string& name, const int max_refresh)
{
    LogTraceFunction;

    // 判断是否存在“选择助战”按钮，存在则点击
    auto screen_choose = ctrler()->get_image();
    RoguelikeRecruitSupportAnalyzer analyzer_choose(screen_choose);
    analyzer_choose.set_mode(SupportAnalyzeMode::ChooseSupportBtn);
    if (!analyzer_choose.analyze()) { // 非开局招募，无助战按钮
        return false;
    }
    const auto& choose_btn_rect = analyzer_choose.get_result_choose_support();
    Log.info(__FUNCTION__, "| check choose support btn ", choose_btn_rect);
    ctrler()->click(choose_btn_rect);
    ProcessTask(*this, { "RoguelikeRecruitSupportEnterFlag" }).run(); // 等待页面加载

    // 识别所有干员，应该最多两页
    const int MaxPageCnt = 2;

    std::vector<RecruitSupportCharInfo> satisfied_chars;
    for (int retry = 0; retry <= max_refresh; ++retry) {
        for (int page = 0; page < MaxPageCnt; ++page) {
            auto screen_char = ctrler()->get_image();
            RoguelikeRecruitSupportAnalyzer analyzer_char(screen_char);
            analyzer_char.set_mode(SupportAnalyzeMode::AnalyzeChars);
            analyzer_char.set_required({ name });
            if (analyzer_char.analyze()) {
                auto& chars_page = analyzer_char.get_result_char();

                bool use_nonfriend_support = m_config->get_use_nonfriend_support();
                auto check_satisfy = [&use_nonfriend_support](const RecruitSupportCharInfo& chara) {
                    return chara.is_friend || use_nonfriend_support;
                };
                std::copy_if(
                    chars_page.begin(),
                    chars_page.end(),
                    std::inserter(satisfied_chars, std::begin(satisfied_chars)),
                    check_satisfy);

                if (satisfied_chars.size()) {
                    break;
                }
            }
            if (page != MaxPageCnt - 1) {
                ProcessTask(*this, { "RoguelikeSupportSwipeRight" }).run();
            }
        }
        if (satisfied_chars.size()) {
            break;
        }

        // 刷新助战
        if (retry >= max_refresh) {
            break;
        }
        auto screen_refresh = ctrler()->get_image();
        RoguelikeRecruitSupportAnalyzer analyzer_refresh(screen_refresh);
        analyzer_refresh.set_mode(SupportAnalyzeMode::RefreshSupportBtn);
        if (!analyzer_refresh.analyze()) {
            click_return_button();
            return false;
        }
        auto& refresh_info = analyzer_refresh.get_result_refresh();
        if (refresh_info.in_cooldown) {
            sleep(refresh_info.remain_secs * 1000);
        }
        ctrler()->click(refresh_info.rect);
        sleep(Task.get("RoguelikeRefreshSupportBtnOcr")->post_delay);
        ProcessTask(*this, { "RoguelikeSupportSwipeLeft" }).run();
    }
    if (satisfied_chars.empty()) {
        // 找不到需要的助战干员，返回正常招募逻辑
        Log.info(__FUNCTION__, "| can't find support char `", name, "`");
        click_return_button();
        return false;
    }

    // 点击干员并记录信息
    auto& satisfied_char = satisfied_chars.front();
    select_oper(satisfied_char.oper_info);

    // 确认选择
    ProcessTask(*this, { "Roguelike@RecruitSupportConfirm" }).run();
    return true;
}

bool asst::RoguelikeRecruitTaskPlugin::recruit_own_char()
{
    LogTraceFunction;

    auto core_opt = m_config->get_core_char();
    if (core_opt.empty()) {
        return false;
    }
    m_config->set_core_char("");
    return recruit_appointed_char(core_opt);
}

void asst::RoguelikeRecruitTaskPlugin::select_oper(const battle::roguelike::Recruitment& oper)
{
    Log.info(__FUNCTION__, "| Choose oper:", oper.name, "( elite", oper.elite, "level", oper.level, ")");

    ctrler()->click(oper.rect);

    m_config->status().opers[oper.name] = { .elite = oper.elite, .level = oper.level };
}

void asst::RoguelikeRecruitTaskPlugin::swipe_to_the_left_of_operlist(int loop_times)
{
    for (int i = 0; i != loop_times; ++i) {
        ProcessTask(*this, { "RoguelikeRecruitOperListSwipeToTheLeft" }).run();
    }
    ProcessTask(*this, { "SleepAfterOperListQuickSwipe" }).run();
}

void asst::RoguelikeRecruitTaskPlugin::slowly_swipe(bool to_left, int swipe_dist)
{
    std::string swipe_task_name =
        to_left ? "RoguelikeRecruitOperListSlowlySwipeToTheLeft" : "RoguelikeRecruitOperListSlowlySwipeToTheRight";
    if (!ControlFeat::support(
            ctrler()->support_features(),
            ControlFeat::PRECISE_SWIPE)) { // 不能精准滑动时不使用 swipe_dist 参数
        ProcessTask(*this, { swipe_task_name }).run();
        return;
    }

    if (!to_left) {
        swipe_dist = -swipe_dist;
    }
    auto swipe_task = Task.get(swipe_task_name);
    const Rect& StartPoint = swipe_task->specific_rect;
    ctrler()->swipe(
        StartPoint,
        { StartPoint.x + swipe_dist - StartPoint.width, StartPoint.y, StartPoint.width, StartPoint.height },
        swipe_task->special_params.empty() ? 0 : swipe_task->special_params.at(0),
        (swipe_task->special_params.size() < 2) ? false : swipe_task->special_params.at(1),
        (swipe_task->special_params.size() < 3) ? 1 : swipe_task->special_params.at(2),
        (swipe_task->special_params.size() < 4) ? 1 : swipe_task->special_params.at(3));
    sleep(swipe_task->post_delay);
}
