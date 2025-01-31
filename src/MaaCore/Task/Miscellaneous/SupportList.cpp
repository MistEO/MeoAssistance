#include "SupportList.h"

#include "Config/GeneralConfig.h"
#include "Config/Miscellaneous/BattleDataConfig.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Battle/SupportListAnalyzer.h"
#include "Vision/Matcher.h"

asst::SupportList::SupportList(Assistant* inst, AbstractTask& task) :
    m_inst_helper(inst),
    m_task(task)
{
}

bool asst::SupportList::select_role(const Role role)
{
    LogTraceFunction;

    clear();

    if (std::ranges::find(SUPPORT_UNIT_ROLES, role) == SUPPORT_UNIT_ROLES.end()) {
        Log.error(__FUNCTION__, "| attempting to select unsupported role", enum_to_string(role));
        m_selected_role = Role::Unknown;
        return false;
    }

    if (ProcessTask(
            m_task,
            { enum_to_string(role, true) + "@SupportList-RoleSelected",
              enum_to_string(role, true) + "@SupportList-SelectRole" })
            .run()) {
        m_selected_role = role;
        Log.info(__FUNCTION__, "| Successfully selected role", enum_to_string(role));
        return true;
    }

    m_selected_role = Role::Unknown;
    Log.error(__FUNCTION__, "| Failed to select role", enum_to_string(role));
    return false;
}

bool asst::SupportList::update()
{
    LogTraceFunction;

    // 识别助战列表当前所选的职业并更新 m_selected_role
    if (m_selected_role == Role::Unknown) {
        Log.info(__FUNCTION__, "| Currently selected role is unknown; updating it before proceeding.");
        if (!update_selected_role(m_inst_helper.ctrler()->get_image())) {
            Log.error(__FUNCTION__, "Fail to update selected role; abandoning support list update.");
            return false;
        }
    }

    move_to_list_head();

    for (unsigned page = 0; page < MAX_NUM_PAGES && !m_inst_helper.need_exit(); ++page) {
        // 更新助战列表失败
        if (const std::optional<unsigned> ret = update_page(); !ret.has_value()) [[unlikely]] {
            Log.error(__FUNCTION__, "| Failed to update list with current page.");
            return false;
        }
        // 没有新的助战栏位
        else if (ret.value() == 0) {
            Log.info(__FUNCTION__, "| No new support units were added; end of support list reached.");
            break;
        }

        if (page < MAX_NUM_PAGES - 1) {
            move_forward();
        }
    }

    return true;
}

bool asst::SupportList::use_support_unit(const size_t index, const int skill, const bool max_spec_lvl)
{
    LogTraceFunction;

    // 边界检查
    if (index >= m_list.size()) {
        Log.error(__FUNCTION__, "| Index", index, "exceeds bounds.");
        return false;
    }

    // 点选助战干员
    click_support_unit(index);

    if (skill != 0) {
        if (const int rarity = BattleData.get_rarity(m_list[index].name); max_spec_lvl && rarity >= 3) {
            Matcher max_spec_lvl_analyzer(m_inst_helper.ctrler()->get_image());
            // 4~6 星干员技能专三
            if (rarity > 3) {
                max_spec_lvl_analyzer.set_task_info("SupportList-MaxSpecLvl-" + std::to_string(skill));
            }
            // 3 星干员技能 7 级
            else { // rarity == 3
                max_spec_lvl_analyzer.set_task_info("SupportList-MaxSkillLvl-" + std::to_string(skill));
            }
            if (!max_spec_lvl_analyzer.analyze()) { // 所需技能非专三/7级
                // 取消选择
                ProcessTask(m_task, { "SupportList-Cancel" }).run();
                return false;
            }
        }
        // 选择技能
        ProcessTask(m_task, { "SupportList-SelectSkill-" + std::to_string(skill) }).run();
    }

    // 确认选择
    ProcessTask(m_task, { "SupportList-Confirm" }).run();

    return true;
}

bool asst::SupportList::refresh_list()
{
    LogTraceFunction;

    clear();

    return ProcessTask(m_task, { "SupportList-CooldownThenRefresh" }).run();
}

void asst::SupportList::clear(const bool reset_selected_role)
{
    LogTraceFunction;

    if (reset_selected_role) {
        m_selected_role = Role::Unknown;
    }
    m_list.clear();
    m_name2index.clear();
    m_view_begin = 0;
    m_view_end = 0;
}

bool asst::SupportList::update_selected_role(const cv::Mat& image)
{
    LogTraceFunction;

    Matcher role_analyzer(image);
    for (const Role role : SUPPORT_UNIT_ROLES) {
        role_analyzer.set_task_info(enum_to_string(role, true) + "@SupportList-RoleSelected");
        if (role_analyzer.analyze()) {
            Log.info(__FUNCTION__, "| Currently selected role is", enum_to_string(role));
            m_selected_role = role;
            return true;
        }
    }

    Log.error(__FUNCTION__, "| Fail to recognise currently selected role. Are we really in the support list?");
    return false;
}

std::optional<unsigned> asst::SupportList::update_page()
{
    LogTraceFunction;

    // 获取助战列表页
    SupportListAnalyzer analyzer(m_inst_helper.ctrler()->get_image());
    // 未识别到任何助战干员，极有可能当前已不在助战列表界面
    if (!analyzer.analyze(m_selected_role)) [[unlikely]] {
        Log.error(__FUNCTION__, "| Empty support list page. Are we really in the support list?");
        return std::nullopt;
    }
    const std::vector<SupportUnit>& support_list_page = analyzer.get_result();

    // 我们假设 support_list_page 为一段从左往右连续编号的助战栏位序列，并设置其起点索引以供检查
    // 如果新的助战栏位中间夹了个已知的，那肯定是出问题了
    size_t index = m_list.size();
    const std::string& head_name = support_list_page.front().name;
    if (const auto it = m_name2index.find(head_name); it != m_name2index.end()) {
        index = it->second;
    }
    // update view
    m_view_begin = index;
    m_view_end = index;

    unsigned num_new_items = 0; // 新增助战栏位记数

    for (const SupportUnit& support_unit : support_list_page) {
        // 基于“助战列表中不会有重复名字的干员”的假设，通过干员名判断是否为新助战栏位
        if (const auto it = m_name2index.find(support_unit.name); it == m_name2index.end()) // 新助战栏位
        {
            Log.info(__FUNCTION__, "| New support unit found:", support_unit.name);
            // sanity check for index
            if (index != m_list.size()) {
                Log.error(__FUNCTION__, "| Sanity check for index failed. Expected:", index, "got:", m_list.size());
                return std::nullopt;
            }
            // 添加新助战栏位
            m_list.emplace_back(support_unit);
            m_name2index.emplace(support_unit.name, index);
            // 更新新增助战栏位记数
            ++num_new_items;
        }
        else { // 已知助战栏位
            Log.info(__FUNCTION__, "| Existing support unit found:", support_unit.name);
            // sanity check for index
            if (index != it->second) {
                Log.error(__FUNCTION__, "| Sanity check for index failed. Expected:", index, "got:", it->second);
                return std::nullopt;
            }
            // 仅更新 rect
            m_list.at(index).rect = support_unit.rect;
        }
        // update view
        // ————————————————————————————————————————————————————————————————
        // 根据“support_list_page 为一段从左往右连续编号的助战栏位序列”的假设
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
        "Updated support units with indices",
        std::to_string(m_view_begin) + "~" + std::to_string(m_view_end));
    return num_new_items;
}

bool asst::SupportList::click_support_unit(const size_t index)
{
    LogTraceFunction;

    move_to_support_unit(index);
    m_inst_helper.ctrler()->click(m_list.at(index).rect);
    m_inst_helper.sleep(Config.get_options().task_delay);
    ProcessTask(m_task, { "Stop@LoadingText", "Stop" }).run();

    return true;
}

void asst::SupportList::move_to_list_head()
{
    LogTraceFunction;

    move_backward(MAX_NUM_PAGES - 1);
}

void asst::SupportList::move_forward(const unsigned num_pages)
{
    LogTraceFunction;

    for (unsigned i = 0; i < num_pages; ++i) {
        ProcessTask(m_task, { "SupportList-SwipeToTheRight" }).run();
    }
}

void asst::SupportList::move_backward(const unsigned num_pages)
{
    LogTraceFunction;

    for (unsigned i = 0; i < num_pages; ++i) {
        ProcessTask(m_task, { "SupportList-SwipeToTheLeft" }).run();
    }
}

void asst::SupportList::move_to_support_unit(const size_t index)
{
    LogTraceFunction;

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
