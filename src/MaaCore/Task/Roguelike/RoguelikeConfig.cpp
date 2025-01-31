#include "RoguelikeConfig.h"

#include "Config/TaskData.h"
#include "Utils/Logger.hpp"

bool asst::RoguelikeConfig::verify_and_load_params(const json::value& params)
{
    // ------------------ 肉鸽主题设置 ------------------
    std::string theme = params.get("theme", std::string(RoguelikeTheme::Phantom));
    if (!RoguelikeConfig::is_valid_theme(theme)) {
        Log.error("Unknown roguelike theme", theme);
        return false;
    }

    auto mode = static_cast<RoguelikeMode>(params.get("mode", 0));
    if (!RoguelikeConfig::is_valid_mode(mode, theme)) {
        Log.error(__FUNCTION__, "| Unknown mode", static_cast<int>(mode));
        return false;
    }

    m_theme = theme;
    m_mode = mode;
    if (m_theme != RoguelikeTheme::Phantom) {
        m_difficulty = params.get("difficulty", 0);
    }
    else if (params.contains("difficulty")) {
        Log.error(__FUNCTION__, "| Invalid difficulty for theme", m_theme);
        return false;
    }

    if (mode == RoguelikeMode::Collectible) {
        m_collectible_mode_shopping = params.get("collectible_mode_shopping", false);

        if (m_theme == RoguelikeTheme::Sami) {
            m_first_floor_foldartal = !params.get("first_floor_foldartal", "").empty();
        }
    }

    m_start_with_seed = params.get("start_with_seed", false);
    m_start_with_elite_two = params.get("start_with_elite_two", false);
    m_only_start_with_elite_two = params.get("only_start_with_elite_two", false);
    if (mode != RoguelikeMode::Collectible && (m_start_with_elite_two || m_only_start_with_elite_two)) {
        Log.error(__FUNCTION__, "| Invalid mode for start_with_elite_two", static_cast<int>(mode));
        return false;
    }
    if (!m_start_with_elite_two && m_only_start_with_elite_two) {
        Log.error(__FUNCTION__, "| only_start_with_elite_two can only be used together with start_with_elite_two");
        return false;
    }

    // 设置层数选点策略，相关逻辑在 RoguelikeStrategyChangeTaskPlugin
    {
        Task.set_task_base(m_theme + "@Roguelike@Stages", m_theme + "@Roguelike@Stages_default");
        std::string strategy_task = m_theme + "@Roguelike@StrategyChange";
        std::string strategy_task_with_mode = strategy_task + "_mode" + std::to_string(static_cast<int>(mode));
        if (Task.get(strategy_task_with_mode) == nullptr) {
            strategy_task_with_mode = "#none"; // 没有对应的层数选点策略，使用默认策略（避战）
            Log.warn(__FUNCTION__, "No strategy for mode", static_cast<int>(mode));
        }
        Task.set_task_base(strategy_task, strategy_task_with_mode);

        // 点刺成锭分队特殊策略
        if (m_theme == "Sarkaz") {
            auto squad = params.get("squad", "");
            if (m_mode == RoguelikeMode::Investment &&
                (squad == "点刺成锭分队" || (squad == "后勤分队" && m_start_with_seed))) {
                if (squad == "点刺成锭分队") {
                    // 启用特殊策略，联动 RoguelikeRoutingTaskPlugin
                    Task.set_task_base(strategy_task, "Sarkaz@Roguelike@StrategyChange-FastInvestment");
                }
                // 禁用前 2 层的 <思维负荷干员编队> 功能
                Task.set_task_base(
                    "Sarkaz@Roguelike@StageBurdenOperation",
                    "Sarkaz@Roguelike@StageBurdenOperation-None");
            }
            else {
                // 启用前 2 层的 <思维负荷干员编队> 功能
                Task.set_task_base(
                    "Sarkaz@Roguelike@StageBurdenOperation",
                    "Sarkaz@Roguelike@StageBurdenOperation-Start");
            }
        }
    }

    if (m_mode == RoguelikeMode::Investment) {
        bool investment_with_more_score = params.get("investment_with_more_score", false);
        if (!params.contains("investment_with_more_score") && params.contains("investment_enter_second_floor")) {
            investment_with_more_score = params.get("investment_enter_second_floor", true);
            Log.warn("================  DEPRECATED  ================");
            Log.warn(
                "`investment_enter_second_floor` has been deprecated since v5.2.1; Please use "
                "'investment_with_more_score'");
            Log.warn("================  DEPRECATED  ================");
        }
        m_invest_with_more_score = (investment_with_more_score);
    }

    if (m_mode == RoguelikeMode::Collectible && !m_only_start_with_elite_two) {
        m_run_for_collectible = true; // 烧开水模式下，如果不是只凹直升，第一轮游戏先烧水
    }

    return true;
}

void asst::RoguelikeConfig::clear()
{
    // ------------------ 通用参数 ------------------
    m_trader_no_longer_buy = false;
    m_core_char = std::string();
    m_squad = std::string();
    m_team_full_without_rookie = false;
    m_use_support = false;
    m_oper = std::unordered_map<std::string, RoguelikeOper>();
    m_collection = std::vector<std::string>();

    m_hope = 0;
    m_hp = 0;
    m_floor = 0;
    m_formation_upper_limit = 6;

    // ------------------ 萨米主题专用参数 ------------------
    m_chaos = 0;
    m_foldartal = std::vector<std::string>();

    // ------------------ 萨卡兹主题专用参数 ------------------
    m_idea_count = 0;         // 构想数量
    m_burden_number = 0;      // 负荷
    m_burden_upper_limit = 3; // 负荷上限
}

