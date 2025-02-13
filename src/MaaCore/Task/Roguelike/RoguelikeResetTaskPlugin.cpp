#include "RoguelikeResetTaskPlugin.h"

#include "Utils/Logger.hpp"

bool asst::RoguelikeResetTaskPlugin::verify(AsstMsg msg, const json::value& details) const
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
    if (task_view == "Roguelike@StartExplore") {
        return true;
    }
    else {
        return false;
    }
}

bool asst::RoguelikeResetTaskPlugin::_run()
{
    for (const auto& plugin : m_task_ptr->get_plugins()) {
        if (auto ptr = std::dynamic_pointer_cast<AbstractRoguelikeTaskPlugin>(plugin)) {
            ptr->reset_in_run_variables();
        }
    }
    m_config->clear();
    Log.flush();
    return true;
}
