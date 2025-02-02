#include "RoguelikeStageEncounterConfig.h"

#include <meojson/json.hpp>

#include "Utils/Logger.hpp"

bool asst::RoguelikeStageEncounterConfig::parse(const json::value& json)
{
    LogTraceFunction;

    const std::string theme = json.at("theme").as_string();

    // 未指定适用模式（即 mode 字段为空）的配置将作为默认配置，以 (theme, DEFAULT_MODE_PLACEHOLDER) 为 key
    // 对于具体模式 mode，其专用配置，以 (theme, mode) 为 key，将继承默认配置
    const std::pair<std::string, int> default_config_key = std::make_pair(theme, DEFAULT_MODE_PLACEHOLDER);
    std::vector<int> modes = json.get("mode", std::vector<int> {});
    std::unordered_map<std::string, RoguelikeEvent> events;

    // 若 modes 为空，则认为当前正在加载默认配置
    // 否则，认为当前正在加载模式专用配置，复制当前的默认配置作为基础
    if (!modes.empty()) {
        events = m_events.at(default_config_key);
    }

    std::vector<std::string>& event_names = m_event_names[theme];

    for (const auto& event_json : json.at("stage").as_array()) {
        RoguelikeEvent event;
        event.name = event_json.at("name").as_string();
        if (ranges::find(event_names, event.name) == event_names.end()) {
            event_names.emplace_back(event.name);
        }
        event.option_num = event_json.get("option_num", 0);
        event.default_choose = event_json.get("choose", 0);
        if (auto choice_require_opt = event_json.find("choices");
            choice_require_opt && choice_require_opt->is_array()) {
            for (const auto& choice_json : choice_require_opt->as_array()) {
                ChoiceRequire choice;
                choice.name = choice_json.at("name").as_string();
                choice.choose = choice_json.get("choose", -1);
                if (auto requirements_opt = choice_json.find("requirements");
                    requirements_opt && requirements_opt->is_array()) {
                    for (const auto& requirement_json : requirements_opt->as_array()) {
                        auto name = requirement_json.get("name", "");
                        if (name == "Vision") {
                            choice.vision.value = requirement_json.get("value", "");
                            choice.vision.type = parse_comparison_type(requirement_json.get("type", ""));
                        }
                        else if (name == "Relic") {
                            // not supported
                        }
                    }
                }
                event.choice_require.emplace_back(std::move(choice));
            }
        }
        events[event.name] = std::move(event);
    }

    // 注意，理论上默认配置与具体模式的配置都可以被多次加载、覆盖，但我目前没有想到这样做的意义
    if (modes.empty()) { // 存储通用配置
        // 防止重复加载
        if (m_events.contains(default_config_key)) {
            Log.error(__FUNCTION__, "| Default configuration for theme", theme, "has already exist!");
            return false;
        }
        m_events[default_config_key] = events;
    }
    else { // 存储模式专用配置
        for (int mode : modes) {
            const std::pair<std::string, int> key = std::make_pair(theme, mode);
            // 防止重复加载
            if (m_events.contains(key)) {
                Log.error(__FUNCTION__, "| Configuration for theme", theme, "and mode", mode, "has already exist!");
                return false;
            }
            m_events[key] = events;
        }
    }

    return true;
}

bool asst::RoguelikeStageEncounterConfig::set_event(
    const std::string& theme,
    const RoguelikeMode mode,
    const std::string& event_name,
    const int choose,
    const int option_num)
{
    std::pair<std::string, int> key = std::make_pair(theme, static_cast<int>(mode));
    if (theme == "Sarkaz" || theme == "Sami") {
        // 在调试器里发现 m_events 中，Sami 和 Sarkaz 的 mode 只有 -1
        key.second = -1;
    }
    // 边界检查
    auto outerIt = m_events.find(key);
    if (outerIt == m_events.end()) {
        return false;
    }
    auto& innerMap = outerIt->second;
    auto innerIt = innerMap.find(event_name);
    if (innerIt == innerMap.end()) {
        return false;
    }
    // 修改事件选择
    m_events[key][event_name].default_choose = choose;
    m_events[key][event_name].option_num = option_num;
    return true;
}

asst::RoguelikeStageEncounterConfig::ComparisonType
    asst::RoguelikeStageEncounterConfig::parse_comparison_type(const std::string& type_str)
{
    if (type_str == ">") {
        return ComparisonType::GreaterThan;
    }
    if (type_str == "<") {
        return ComparisonType::LessThan;
    }
    if (type_str == "=") {
        return ComparisonType::Equal;
    }
    return ComparisonType::Unsupported;
}
