#include "RoguelikeLastRewardTaskPlugin.h"

#include "Config/TaskData.h"
#include "Status.h"
#include "Task/ProcessTask.h"
#include "Task/Roguelike/RoguelikeLastRewardSelectTaskPlugin.h"
#include "Utils/Logger.hpp"

bool asst::RoguelikeLastRewardTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
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
    if (task_view == "Roguelike@ExitThenAbandon_ToHardest") {
        // 打完低难度的三层，重开高难度烧水壶
        return true;
    }
    else {
        return false;
    }
}

bool asst::RoguelikeLastRewardTaskPlugin::_run()
{
    LogTraceFunction;

    auto mode = m_config->get_mode();
    std::string stages_task_name = m_config->get_theme() + "@Roguelike@Stages";
    std::string strategy_task_name = stages_task_name + "_default";

    if (Task.get(strategy_task_name) == nullptr) {
        Log.error("Strategy task", strategy_task_name, "doesn't exist!");
    }
    else {
        // 重置选点策略
        Task.set_task_base(stages_task_name, strategy_task_name);
    }

    if (m_config->get_theme() != RoguelikeTheme::Phantom && mode == RoguelikeMode::Collectible) {
        // 关闭烧开水 Flag，将难度调整回用户设置的数值
        m_config->set_run_for_collectible(false);
    }
    return true;
}
