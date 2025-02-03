#include "RoguelikeFoldartalStartTaskPlugin.h"

#include "Utils/Logger.hpp"

bool asst::RoguelikeFoldartalStartTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    // 如果正在烧水就跳过
    if (m_config->get_run_for_collectible()) {
        return false;
    }

    const std::string roguelike_name = m_config->get_theme() + "@";
    const std::string& task = details.get("details", "task", "");
    std::string_view task_view = task;

    if (task_view.starts_with(roguelike_name)) {
        task_view.remove_prefix(roguelike_name.length());
    }
    if (task_view == "Roguelike@LastRewardChoose") {
        return true;
    }
    else {
        return false;
    }
}

bool asst::RoguelikeFoldartalStartTaskPlugin::load_params(const json::value& params)
{
    // 本插件仅在萨米肉鸽下烧开水模式中检测到 start_foldartal_list 参数且为非空时启用
    if (m_config->get_theme() != RoguelikeTheme::Sami || m_config->get_mode() != RoguelikeMode::Collectible ||
        !params.contains("start_foldartal_list")) {
        return false;
    }

    if (auto opt = params.find<json::array>("start_foldartal_list"); opt) {
        std::vector<std::string> list;
        for (const auto& name : *opt) {
            if (std::string name_str = name.as_string(); !name_str.empty()) {
                list.emplace_back(name_str);
            }
        }
        /* 由于插件 load_param返回值仅决定自身是否启用，参数验证移动至他处 */
        /*
        if (list.empty()) {
            Log.error(__FUNCTION__, "| Empty start_foldartal_list");
            return false;
        }
        */
        m_start_foldartal_list = std::move(list);
    }

    return !m_start_foldartal_list.empty();
}

bool asst::RoguelikeFoldartalStartTaskPlugin::_run()
{
    LogTraceFunction;

    auto mode = m_config->get_mode();
    bool start_foldartal_checked = check_foldartals();

    // 没有刷到需要的板子，退出重开
    if (mode == RoguelikeMode::Collectible && !start_foldartal_checked) {
        m_config->set_run_for_collectible(true); // 重新烧水
    }
    return true;
}

bool asst::RoguelikeFoldartalStartTaskPlugin::check_foldartals()
{
    const auto& all_foldartal = m_config->status().foldartal_list;

    // 查找板子
    for (const auto& foldartal : m_start_foldartal_list) {
        if (std::find(all_foldartal.begin(), all_foldartal.end(), foldartal) == all_foldartal.end()) {
            // 如果板子没有找到，返回false
            return false;
        }
    }

    // 如果所有板子都找到了，停止
    return true;
}
