#include "RoguelikeShoppingTaskPlugin.h"

#include "Config/Miscellaneous/BattleDataConfig.h"
#include "Config/Roguelike/RoguelikeShoppingConfig.h"
#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Matcher.h"
#include "Vision/OCRer.h"

bool asst::RoguelikeShoppingTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    if (!details.get("details", "task", "").ends_with("Roguelike@TraderRandomShopping")) {
        return false;
    }
    else if (m_config->get_mode() == RoguelikeMode::Investment) {
        return m_config->get_invest_with_more_score();
    }
    else if (m_config->get_mode() == RoguelikeMode::Collectible) {
        return m_config->get_collectible_mode_shopping();
    }
    return true;
}

bool asst::RoguelikeShoppingTaskPlugin::_run()
{
    LogTraceFunction;

    buy_once();
    const auto& theme = m_config->get_theme();
    if ((theme == RoguelikeTheme::Sami || theme == RoguelikeTheme::Sarkaz) &&
        m_config->get_mode() == RoguelikeMode::Exp) {
        // 点击刷新
        ProcessTask(*this, { theme + "@Roguelike@StageTraderRefresh" }).run();
        buy_once();
    }

    return true;
}

bool asst::RoguelikeShoppingTaskPlugin::buy_once()
{
    LogTraceFunction;

    auto image = ctrler()->get_image();
    OCRer analyzer(image);
    analyzer.set_task_info("RoguelikeTraderShoppingOcr");
    if (!analyzer.analyze()) {
        return false;
    }

    bool no_longer_buy = m_config->get_trader_no_longer_buy();

    std::unordered_map<battle::Role, size_t> map_roles_count;
    std::unordered_map<battle::Role, size_t> map_wait_promotion;
    size_t total_wait_promotion = 0;
    std::unordered_set<std::string> chars_list;
    for (const auto& [name, oper] : m_config->status().opers) {
        int elite = oper.elite;
        int level = oper.level;
        Log.info(name, elite, level);

        // 等级太低的干员没必要为他专门买收藏品什么的
        if (level < 60) {
            continue;
        }

        chars_list.emplace(name);

        if (name == "阿米娅") {
            map_roles_count[battle::Role::Caster] += 1;
            map_roles_count[battle::Role::Warrior] += 1;
            map_roles_count[battle::Role::Medic] += 1;
            if (elite == 1 && level == 70) {
                total_wait_promotion += 1;
                map_wait_promotion[battle::Role::Caster] += 1;
                map_wait_promotion[battle::Role::Warrior] += 1;
                map_wait_promotion[battle::Role::Medic] += 1;
            }
        }
        else {
            battle::Role role = BattleData.get_role(name);
            map_roles_count[role] += 1;

            static const std::unordered_map<int, int> RarityPromotionLevel = {
                { 0, INT_MAX }, { 1, INT_MAX }, { 2, INT_MAX }, { 3, INT_MAX }, { 4, 60 }, { 5, 70 }, { 6, 80 },
            };
            int rarity = BattleData.get_rarity(name);
            if (elite == 1 && level >= RarityPromotionLevel.at(rarity)) {
                total_wait_promotion += 1;
                map_wait_promotion[role] += 1;
            }
        }
    }

    const auto& raw_result = analyzer.get_result();
    std::vector<TextRect> result;
    Matcher matcher_analyzer;
    matcher_analyzer.set_image(image);
    matcher_analyzer.set_task_info("RoguelikeTraderShopping");
    for (auto& item : raw_result) {
        matcher_analyzer.set_roi(item.rect.move({ -20, 130, 200, 80 }));
        if (matcher_analyzer.analyze()) {
            result.emplace_back(item);
        }
    }

    // bool bought = false;
    auto& all_goods = RoguelikeShopping.get_goods(m_config->get_theme());
    std::vector<std::string> all_foldartal = m_config->get_theme() == RoguelikeTheme::Sami
                                                 ? Task.get<OcrTaskInfo>("Sami@Roguelike@FoldartalGainOcr")->text
                                                 : std::vector<std::string>();
    for (const auto& goods : all_goods) {
        if (need_exit()) {
            return false;
        }
        if (no_longer_buy && !goods.ignore_no_longer_buy) {
            continue;
        }

        // 在萨米肉鸽刷坍缩范式时不再购买会减少坍缩值的藏品
        if (m_config->get_mode() == RoguelikeMode::CLP_PDS && goods.decrease_collapse) {
            continue;
        }

        auto find_it = ranges::find_if(result, [&](const TextRect& tr) -> bool {
            return tr.text.find(goods.name) != std::string::npos || goods.name.find(tr.text) != std::string::npos;
        });
        if (find_it == result.cend()) {
            continue;
        }

        if (!goods.roles.empty()) {
            bool role_matched = false;
            for (const auto& role : goods.roles) {
                if (map_roles_count[role] != 0) {
                    role_matched = true;
                    break;
                }
            }
            if (!role_matched) {
                Log.trace("Ready to buy", goods.name, ", but there is no such professional operator, skip");
                continue;
            }
        }

        if (goods.promotion != 0) {
            if (total_wait_promotion == 0) {
                Log.trace("Ready to buy", goods.name, ", but there is no one waiting for promotion, skip");
                continue;
            }
            if (!goods.roles.empty()) {
                bool role_matched = false;
                for (const auto& role : goods.roles) {
                    if (map_wait_promotion[role] != 0) {
                        role_matched = true;
                        break;
                    }
                }
                if (!role_matched) {
                    Log.trace("Ready to buy", goods.name, ", but there is no one waiting for promotion, skip");
                    continue;
                }
            }
        }

        if (!goods.chars.empty()) {
            if (ranges::find_first_of(chars_list, goods.chars) == chars_list.cend()) {
                Log.trace("Ready to buy", goods.name, ", but there is no such character, skip");
                continue;
            }
        }

        // 这里仅点一下收藏品，原本的 ProcessTask 还会再点一下，但它是由 rect_move
        // 的，保证不会点出去 即 ProcessTask 多点的那一下会点到不影响的地方 然后继续走 next 里确认
        // or 取消等等的逻辑
        Log.info("Ready to buy", goods.name);
        ctrler()->click(find_it->rect);
        // bought = true;
        if (m_config->get_theme() == RoguelikeTheme::Sami) {
            auto iter = std::find(all_foldartal.begin(), all_foldartal.end(), goods.name);
            if (iter != all_foldartal.end()) {
                // 把goods.name存到密文板overview里
                m_config->status().foldartal_list.emplace_back(goods.name);
            }
        }
        // 把goods.name存到已获得藏品里
        m_config->status().collections.emplace_back(goods.name);
        if (goods.no_longer_buy) {
            m_config->set_trader_no_longer_buy(true);
        }
        break;
    }
    /*
    if (!bought) {
        // 如果什么都没买，即使有商品，说明也是不需要买的，这里强制离开商店，后面让 ProcessTask
    继续跑 return ProcessTask(*this, { "RoguelikeTraderShoppingOver" }).run();
    }
    */
    return true;
}

