#include "OperList.h"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Battle/OperListAnalyzer.h"
#include "Vision/Matcher.h"

asst::OperList::OperList(Assistant* inst, AbstractTask& task) :
    m_inst_helper(inst),
    m_task(task)
{
}

bool asst::OperList::expand_role_list()
{
    LogTraceFunction;

    ProcessTask(m_task, { "OperList-ExpandRoleList", "Stop" }).run();

    if (update_selected_role(m_inst_helper.ctrler()->get_image())) {
        m_role_list_expanded = true;
        Log.info(__FUNCTION__, "| Role list is now expanded.");
        return true;
    }

    Log.error(__FUNCTION__, "| Fail to expand role list.");
    return false;
}

bool asst::OperList::select_role(const Role role, const bool force)
{
    LogTraceFunction;

    // 检查参数有效性
    if (std::ranges::find(OPER_ROLES, role) == OPER_ROLES.end()) {
        Log.error(__FUNCTION__, "| Attempting to select unsupported role", enum_to_string(role));
        m_selected_role = Role::Unknown;
        return false;
    }

    // 确保干员职业筛选列表已展开，注意此处 expand_role_list() 同时通过 update_selected_role() 更新了 m_selected_role。
    if (!m_role_list_expanded && !expand_role_list() || !update_selected_role(m_inst_helper.ctrler()->get_image())) {
        Log.error(__FUNCTION__, "| Fail to recognise expanded role list.");
        return false;
    };

    // 当前所选职业已经为 role 且无须重新选择职业
    if (role == m_selected_role && !force) {
        Log.info(
            __FUNCTION__,
            "| Currently selected role has already been",
            role == ROLE_ALL ? "ALL" : enum_to_string(role));
        return true;
    }

    clear();

    // ———————— ALL ————————
    if (role == ROLE_ALL) {
        // 若当前所选职业已经为 “全部” 但需要重新选择职业，则先点选一下 “先锋”
        if (role == m_selected_role && force) {
            ProcessTask(m_task, { "Pioneer@OperList-SelectRole" }).run();
        }

        if (ProcessTask(m_task, { "All@OperList-SelectRole", "All@OperList-SelectRole-OCR" }).run()) {
            Log.info(__FUNCTION__, "| Successfully selected role ALL");
            return true;
        }

        m_selected_role = Role::Unknown;
        Log.error(__FUNCTION__, "| Failed to select role ALL");
        return false;
    }

    // ———————— 特定职业 ————————
    // 若当前所选职业已经为特定职业 role 但需要重新选择职业，则先点选一下 “全部”
    if (role == m_selected_role && force) {
        ProcessTask(m_task, { "ALL@OperList-SelectRole", "All@OperList-SelectRole-OCR" }).run();
    }

    if (ProcessTask(m_task, { enum_to_string(role, true) + "@OperList-SelectRole" }).run()) {
        Log.info(__FUNCTION__, "| Successfully selected role", enum_to_string(role));
        return true;
    }

    m_selected_role = Role::Unknown;
    Log.error(__FUNCTION__, "| Failed to select role", enum_to_string(role));
    return false;
}

void asst::OperList::sort(const SortKey sort_key, const bool ascending)
{
    LogTraceFunction;

    clear();

    bool custom_sort_expanded = false; // 自定义排序是否已展开

    // 根据需要，展开自定义排序
    if (is_custom_sort_key(sort_key)) {
        ProcessTask(m_task, { "OperList-ExpandCustomSort" }).run();
        custom_sort_expanded = true;
    }

    // 先点击一下所选的排序键，若之前已经选择该排序键，则会切换升序/降序——无论如何，都会重新排序。
    ProcessTask(m_task, { enum_to_string(sort_key) + "@OperList-Sort" }).run();

    // 根据需要切换到升序
    if (ascending) {
        ProcessTask(m_task, { "OperList-NonCustomSort-SetAsc", "OperList-CustomSort-SetAsc", "Stop" }).run();
    }

    // 折叠已展开的自定义排序
    if (custom_sort_expanded) {
        ProcessTask(m_task, { "OperList-CollapseCustomSort" }).run();
        custom_sort_expanded = false;
    }

    m_sort_key = sort_key;
    m_ascending = ascending;

    Log.info(
        __FUNCTION__,
        "| Operator list is now sorted by",
        enum_to_string(sort_key),
        "in",
        ascending ? "ascending" : "descending",
        "order");
}

bool asst::OperList::pin_marked_opers()
{
    LogTraceFunction;

    clear();

    if (ProcessTask(m_task, { "OperList-MarkedOpersPinned", "OperList-PinMarkedOpers" }).run()) {
        Log.info(__FUNCTION__, "| Successfully pinned marked opers.");
        return true;
    }

    Log.error(__FUNCTION__, "| Failed to pin marked operators.");
    return false;
}

bool asst::OperList::unpin_marked_opers()
{
    LogTraceFunction;

    clear();

    if (ProcessTask(m_task, { "OperList-MarkedOpersUnpinned", "OperList-UnpinMarkedOpers" }).run()) {
        Log.info(__FUNCTION__, "| Successfully unpinned marked operators.");
        return true;
    }

    Log.error(__FUNCTION__, "| Failed to unpin marked operators.");
    return false;
}

void asst::OperList::clear(const bool full_reset)
{
    LogTraceFunction;

    if (full_reset) {
        m_role_list_expanded = false;
        m_selected_role = Role::Unknown;
        m_sort_key = SortKey::Level;
        m_ascending = false;
    }
    m_list.clear();
    m_name2index.clear();
    m_view_begin = 0;
    m_view_end = 0;
}

std::optional<unsigned> asst::OperList::update_page()
{
    LogTraceFunction;

    // 获取助战列表页
    OperListAnalyzer analyzer(m_inst_helper.ctrler()->get_image());
    // 未识别到任何助战干员，极有可能当前已不在助战列表界面
    if (!analyzer.analyze()) [[unlikely]] {
        Log.error(__FUNCTION__, "| Empty operator list page. Are we really in the operator list?");
        return std::nullopt;
    }
    const std::vector<CandidateOper>& oper_list_page = analyzer.get_result();

    // 我们假设 oper_list_page 为一段从左往右从上往下连续编号的干员栏位序列，并设置其起点索引以供检查
    // 如果新的干员栏位中间夹了个已知的，那肯定是出问题了
    size_t index = m_list.size();
    const std::string& head_name = oper_list_page.front().name;
    if (const auto it = m_name2index.find(head_name); it != m_name2index.end()) {
        index = it->second;
    }
    // update view
    m_view_begin = index;
    m_view_end = index;

    unsigned num_new_items = 0; // 新增助战栏位记数

    for (const CandidateOper& candidate_oper : oper_list_page) {
        // 基于“助战列表中不会有重复名字的干员”的假设，通过干员名判断是否为新助战栏位
        if (const auto it = m_name2index.find(candidate_oper.name); it == m_name2index.end()) // 新助战栏位
        {
            Log.info(__FUNCTION__, "| New operator found:", candidate_oper.name);
            // sanity check for index
            if (index != m_list.size()) {
                Log.error(__FUNCTION__, "| Sanity check for index failed. Expected:", index, "got:", m_list.size());
                return std::nullopt;
            }
            // 添加新干员栏位
            m_list.emplace_back(candidate_oper);
            m_name2index.emplace(candidate_oper.name, index);
            // 更新新增干员栏位记数
            ++num_new_items;
        }
        else { // 已知助战栏位
            Log.info(__FUNCTION__, "| Existing support unit found:", candidate_oper.name);
            // sanity check for index
            if (index != it->second) {
                Log.error(__FUNCTION__, "| Sanity check for index failed. Expected:", index, "got:", it->second);
                return std::nullopt;
            }
            // 仅更新 rect
            m_list.at(index).rect = candidate_oper.rect;
        }
        // update view
        // ————————————————————————————————————————————————————————————————
        // 根据“oper_list_page 为一段从左往右连续编号的干员栏位序列”的假设
        // 加上 sanity check，可以确保 index 只会递增 因而无需考虑 m_view_begin
        // ————————————————————————————————————————————————————————————————
        // if (index < m_view_begin) {
        //     m_view_begin = index;
        // }
        if (index >= m_view_end) {
            m_view_end = index + 1;
        }
        ++index;
    }

    Log.info(
        __FUNCTION__,
        "Updated operators with indices",
        std::to_string(m_view_begin) + "~" + std::to_string(m_view_end));
    return num_new_items;
}

bool asst::OperList::select_oper(const size_t index)
{
    LogTraceFunction;

    // 边界检查
    if (index >= m_list.size()) {
        Log.error(__FUNCTION__, "| Index", index, "is out of bound. Current list size is", m_list.size());
        return false;
    }

    // 根据需要滑动页面
    if (index >= m_view_end || index < m_view_begin) {
        move_to_oper(index);
    }

    if (m_list[index].selected) {
        return true;
    }

    m_inst_helper.ctrler()->click(m_list[index].rect);

    // 判断干员的选择状态是否改变
    if (update_selected_status(index)) {
        return true;
    }

    Log.error(__FUNCTION__, "| The operator's selected status remains unchanged after being clicked.");
    return false;
}

bool asst::OperList::unselect_oper(const size_t index)
{
    LogTraceFunction;

    // 边界检查
    if (index >= m_list.size()) {
        Log.error(__FUNCTION__, "| Index", index, "is out of bound. Current list size is", m_list.size());
        return false;
    }

    // 根据需要滑动页面
    if (index >= m_view_end || index < m_view_begin) {
        move_to_oper(index);
    }

    if (!m_list[index].selected) {
        return true;
    }

    m_inst_helper.ctrler()->click(m_list[index].rect);

    // 判断干员的选择状态是否改变
    if (!update_selected_status(index)) {
        return true;
    }

    Log.error(__FUNCTION__, "| The operator's selected status remains unchanged after being clicked.");
    return false;
}

void asst::OperList::move_forward(const unsigned num_pages)
{
    LogTraceFunction;

    for (unsigned i = 0; i < num_pages; ++i) {
        ProcessTask(m_task, { "OperList-SwipeToTheRight" }).run();
    }
}

void asst::OperList::move_backward(const unsigned num_pages)
{
    LogTraceFunction;

    for (unsigned i = 0; i < num_pages; ++i) {
        ProcessTask(m_task, { "OperList-SwipeToTheLeft" }).run();
    }
}

void asst::OperList::move_to_oper(const size_t index)
{
    LogTraceFunction;

    // 边界检查
    if (index >= m_list.size()) {
        Log.error(__FUNCTION__, "| Index", index, "is out of bound. Current list size is", m_list.size());
        return;
    }

    while (index >= m_view_end || index < m_view_begin) {
        if (index >= m_view_end) {
            move_forward();
        }
        else if (index < m_view_begin) {
            move_backward();
        }
        update_page();
    }
}

bool asst::OperList::update_selected_role(const cv::Mat& image)
{
    LogTraceFunction;

    Matcher role_analyzer(image);
    for (const Role role : OPER_ROLES) {
        role_analyzer.set_task_info((role == ROLE_ALL ? "All" : enum_to_string(role, true)) + "@OperList-RoleSelected");
        if (role_analyzer.analyze()) {
            Log.info(__FUNCTION__, "| Currently selected role is", role == ROLE_ALL ? "ALL" : enum_to_string(role));
            m_selected_role = role;
            return true;
        }
    }

    Log.error(__FUNCTION__, "| Fail to recognise currently selected role. Has the role list been expanded?");
    return false;
}

bool asst::OperList::update_selected_status(const size_t index)
{
    LogTraceFunction;

    // 边界检查
    if (index >= m_list.size()) {
        Log.error(__FUNCTION__, "| Index", index, "is out of bound. Current list size is", m_list.size());
        return false;
    }

    // 根据需要滑动页面
    if (index >= m_view_end || index < m_view_begin) {
        move_to_oper(index);
    }

    const auto& selected_task_ptr = Task.get<MatchTaskInfo>("OperListAnalyzer-Selected");

    Matcher selected_analyzer(m_inst_helper.ctrler()->get_image());
    const Rect selected_roi = m_list[index].rect.move(selected_task_ptr->rect_move);
    selected_analyzer.set_task_info(selected_task_ptr);
    selected_analyzer.set_roi(selected_roi);
    m_list[index].selected = selected_analyzer.analyze().has_value();

    return m_list[index].selected;
}
