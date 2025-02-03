#include "RoguelikeTask.h"

#include "Common/AsstBattleDef.h"
#include "Config/TaskData.h"
#include "Task/ProcessTask.h"

// ------------------ 通用配置及插件 ------------------
#include "Task/Miscellaneous/ScreenshotTaskPlugin.h"
#include "Task/Roguelike/RoguelikeBattleTaskPlugin.h"
#include "Task/Roguelike/RoguelikeConfig.h"
#include "Task/Roguelike/RoguelikeControlTaskPlugin.h"
#include "Task/Roguelike/RoguelikeCustomStartTaskPlugin.h"
#include "Task/Roguelike/RoguelikeDebugTaskPlugin.h"
#include "Task/Roguelike/RoguelikeDifficultySelectionTaskPlugin.h"
#include "Task/Roguelike/RoguelikeFormationTaskPlugin.h"
#include "Task/Roguelike/RoguelikeInvestTaskPlugin.h"
#include "Task/Roguelike/RoguelikeIterateDeepExplorationPlugin.h"
#include "Task/Roguelike/RoguelikeIterateMonthlySquadPlugin.h"
#include "Task/Roguelike/RoguelikeLastRewardSelectTaskPlugin.h"
#include "Task/Roguelike/RoguelikeLastRewardTaskPlugin.h"
#include "Task/Roguelike/RoguelikeLevelTaskPlugin.h"
#include "Task/Roguelike/RoguelikeRecruitTaskPlugin.h"
#include "Task/Roguelike/RoguelikeResetTaskPlugin.h"
#include "Task/Roguelike/RoguelikeSettlementTaskPlugin.h"
#include "Task/Roguelike/RoguelikeShoppingTaskPlugin.h"
#include "Task/Roguelike/RoguelikeSkillSelectionTaskPlugin.h"
#include "Task/Roguelike/RoguelikeStageEncounterTaskPlugin.h"
#include "Task/Roguelike/RoguelikeStrategyChangeTaskPlugin.h"

// ------------------ 萨米主题专用配置及插件 ------------------
#include "Config/Roguelike/Sami/RoguelikeCollapsalParadigmConfig.h"
#include "Task/Roguelike/Sami/RoguelikeCollapsalParadigmTaskPlugin.h"
#include "Task/Roguelike/Sami/RoguelikeFoldartalGainTaskPlugin.h"
#include "Task/Roguelike/Sami/RoguelikeFoldartalStartTaskPlugin.h"
#include "Task/Roguelike/Sami/RoguelikeFoldartalUseTaskPlugin.h"

// ------------------ 萨卡兹主题专用配置及插件 ------------------
#include "Task/Roguelike/Map/RoguelikeRoutingTaskPlugin.h"
#include "Task/Roguelike/RoguelikeInputSeedTaskPlugin.h"

#include "Utils/Logger.hpp"

asst::RoguelikeTask::RoguelikeTask(const AsstCallback& callback, Assistant* inst) :
    InterfaceTask(callback, inst, TaskType),
    m_roguelike_task_ptr(std::make_shared<ProcessTask>(callback, inst, TaskType)),
    m_config_ptr(std::make_shared<RoguelikeConfig>())
{
    LogTraceFunction;

    m_roguelike_task_ptr->set_ignore_error(true);
    m_control_ptr = m_roguelike_task_ptr->register_plugin<RoguelikeControlTaskPlugin>(m_config_ptr);

    // ------------------ 通用插件 ------------------
    m_roguelike_task_ptr->register_plugin<ScreenshotTaskPlugin>();
    m_roguelike_task_ptr->register_plugin<RoguelikeFormationTaskPlugin>(m_config_ptr, m_control_ptr);
    m_roguelike_task_ptr->register_plugin<RoguelikeResetTaskPlugin>(m_config_ptr, m_control_ptr);
    m_roguelike_task_ptr->register_plugin<RoguelikeSettlementTaskPlugin>(m_config_ptr, m_control_ptr);
    m_invest_ptr = m_roguelike_task_ptr->register_plugin<RoguelikeInvestTaskPlugin>(m_config_ptr, m_control_ptr);

    m_debug_ptr = m_roguelike_task_ptr->register_plugin<RoguelikeDebugTaskPlugin>(m_config_ptr, m_control_ptr);
    m_custom_ptr = m_roguelike_task_ptr->register_plugin<RoguelikeCustomStartTaskPlugin>(m_config_ptr, m_control_ptr);
    m_roguelike_task_ptr->register_plugin<RoguelikeShoppingTaskPlugin>(m_config_ptr, m_control_ptr)->set_retry_times(0);

    m_roguelike_task_ptr->register_plugin<RoguelikeBattleTaskPlugin>(m_config_ptr, m_control_ptr)
        ->set_retry_times(0)
        .set_ignore_error(true);
    m_roguelike_task_ptr->register_plugin<RoguelikeRecruitTaskPlugin>(m_config_ptr, m_control_ptr)
        ->set_retry_times(2)
        .set_ignore_error(true);

    m_roguelike_task_ptr->register_plugin<RoguelikeSkillSelectionTaskPlugin>(m_config_ptr, m_control_ptr)
        ->set_retry_times(2)
        .set_ignore_error(true);

    m_roguelike_task_ptr->register_plugin<RoguelikeStageEncounterTaskPlugin>(m_config_ptr, m_control_ptr)
        ->set_retry_times(0);

    m_roguelike_task_ptr->register_plugin<RoguelikeLastRewardTaskPlugin>(m_config_ptr, m_control_ptr);
    m_roguelike_task_ptr->register_plugin<RoguelikeLastRewardSelectTaskPlugin>(m_config_ptr, m_control_ptr);

    m_roguelike_task_ptr->register_plugin<RoguelikeDifficultySelectionTaskPlugin>(m_config_ptr, m_control_ptr);
    m_roguelike_task_ptr->register_plugin<RoguelikeStrategyChangeTaskPlugin>(m_config_ptr, m_control_ptr);

    m_roguelike_task_ptr->register_plugin<RoguelikeIterateMonthlySquadPlugin>(m_config_ptr, m_control_ptr)
        ->set_retry_times(3);
    m_roguelike_task_ptr->register_plugin<RoguelikeIterateDeepExplorationPlugin>(m_config_ptr, m_control_ptr)
        ->set_retry_times(3);

    m_roguelike_task_ptr->register_plugin<RoguelikeLevelTaskPlugin>(m_config_ptr, m_control_ptr);

    // ------------------ 萨米主题专用插件 ------------------
    m_roguelike_task_ptr->register_plugin<RoguelikeFoldartalGainTaskPlugin>(m_config_ptr, m_control_ptr);
    m_foldartal_use_ptr =
        m_roguelike_task_ptr->register_plugin<RoguelikeFoldartalUseTaskPlugin>(m_config_ptr, m_control_ptr);
    m_foldartal_start_ptr =
        m_roguelike_task_ptr->register_plugin<RoguelikeFoldartalStartTaskPlugin>(m_config_ptr, m_control_ptr);
    m_roguelike_task_ptr->register_plugin<RoguelikeCollapsalParadigmTaskPlugin>(m_config_ptr, m_control_ptr);

    // ------------------ 萨卡兹主题专用插件 ------------------
    m_roguelike_task_ptr->register_plugin<RoguelikeRoutingTaskPlugin>(m_config_ptr, m_control_ptr);
    m_roguelike_task_ptr->register_plugin<RoguelikeInputSeedTaskPlugin>(m_config_ptr, m_control_ptr);

    // 这个任务如果卡住会放弃当前的肉鸽并重新开始，所以多添加亿点。先这样凑合用
    for (int i = 0; i != 999; ++i) {
        m_subtasks.emplace_back(m_roguelike_task_ptr);
    }
}

bool asst::RoguelikeTask::set_params(const json::value& params)
{
    LogTraceFunction;
    if (!m_config_ptr->verify_and_load_params(params)) {
        m_roguelike_task_ptr->set_tasks({ "Stop" });
        return false;
    }

    const auto& theme = m_config_ptr->get_theme();
    const auto& mode = m_config_ptr->get_mode();

    m_roguelike_task_ptr->set_tasks({ theme + "@Roguelike@Begin" });

    if (mode == RoguelikeMode::Investment) {
        // 刷源石锭模式是否进入第二层
        if (m_config_ptr->get_invest_with_more_score()) {
            // 战斗后奖励默认
            Task.set_task_base(theme + "@Roguelike@DropsFlag", theme + "@Roguelike@DropsFlag_default");
            m_roguelike_task_ptr->set_times_limit("StageTraderInvestCancel", INT_MAX);
            m_roguelike_task_ptr->set_times_limit("StageTraderLeaveConfirm", 0, ProcessTask::TimesLimitType::Post);
        }
        else {
            // 战斗后奖励只拿钱
            Task.set_task_base(theme + "@Roguelike@DropsFlag", theme + "@Roguelike@DropsFlag_mode1");
            m_roguelike_task_ptr->set_times_limit("StageTraderInvestCancel", 0);
            m_roguelike_task_ptr->set_times_limit("StageTraderLeaveConfirm", INT_MAX);
        }
    }
    else {
        // 重置战斗后奖励next
        Task.set_task_base(theme + "@Roguelike@DropsFlag", theme + "@Roguelike@DropsFlag_default");
        m_roguelike_task_ptr->set_times_limit("StageTraderInvestCancel", INT_MAX);
        m_roguelike_task_ptr->set_times_limit("StageTraderLeaveConfirm", INT_MAX);
    }

    bool stop_at_final_boss = params.get("stop_at_final_boss", false);
    // 傀影肉鸽3层和5层boss图标一样,禁用
    if (stop_at_final_boss && theme != RoguelikeTheme::Phantom) {
        m_roguelike_task_ptr->set_times_limit(theme + "@Roguelike@StageDreadfulFoe-5", 0);
    }
    else {
        // 重置boss进点
        m_roguelike_task_ptr->set_times_limit(theme + "@Roguelike@StageDreadfulFoe-5", INT_MAX);
    }

    if (theme == RoguelikeTheme::Sami) {
        if (auto opt = params.find<json::array>("start_foldartal_list"); opt) {
            std::vector<std::string> list;
            for (const auto& name : *opt) {
                if (std::string name_str = name.as_string(); !name_str.empty()) {
                    list.emplace_back(name_str);
                }
            }
            /* 由于插件 load_param返回值仅决定自身是否启用，二次读取参数进行验证 */
            if (list.empty()) {
                Log.error(__FUNCTION__, "| Empty start_foldartal_list");
                return false;
            }
        }
    }
    if (theme == RoguelikeTheme::Sarkaz) {
        // 刷等级选择美愿
        if (params.get("choose_beautiful_wish", false)) {
            RoguelikeStageEncounter.set_event(theme, mode, "相遇", 3, 4);
        }
        else {
            // 若取消勾选，需要重新设为希望时代的涂鸦
            RoguelikeStageEncounter.set_event(theme, mode, "相遇", 1, 4);
        }
    }

    m_roguelike_task_ptr->set_times_limit(theme + "@Roguelike@StartExplore", params.get("starts_count", INT_MAX));
    // 通过 exceededNext 禁用投资系统，进入商店购买逻辑
    m_roguelike_task_ptr->set_times_limit(
        "StageTraderInvestSystem",
        params.get("investment_enabled", true) ? INT_MAX : 0);
    m_roguelike_task_ptr->set_times_limit(
        "StageTraderRefreshWithDice",
        params.get("refresh_trader_with_dice", false) ? INT_MAX : 0);

    for (const auto& plugin : m_roguelike_task_ptr->get_plugins()) {
        if (const auto& p_ptr = std::dynamic_pointer_cast<AbstractRoguelikeTaskPlugin>(plugin); p_ptr != nullptr) {
            p_ptr->set_enable(p_ptr->load_params(params));
        }
    }

    return true;
}
