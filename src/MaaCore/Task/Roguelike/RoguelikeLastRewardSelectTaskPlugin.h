#pragma once
#include "AbstractRoguelikeTaskPlugin.h"

namespace asst
{

struct RoguelikeStartSelect // 刷开局模式下凹开局奖励选择
{
    bool hot_water = false; // 热水壶
    bool shield = false;    // 盾；傀影没盾，是生命
    bool ingot = false;     // 源石锭
    bool hope = false;      // 希望
    bool random = false;    // 随机奖励
    bool key = false;       // 钥匙
    bool dice = false;      // 骰子
    bool ideas = false;     // 构想
};

class RoguelikeLastRewardSelectTaskPlugin : public AbstractRoguelikeTaskPlugin
{
public:
    using AbstractRoguelikeTaskPlugin::AbstractRoguelikeTaskPlugin;
    virtual ~RoguelikeLastRewardSelectTaskPlugin() = default;
    virtual bool load_params(const json::value& params) override;
    virtual bool verify(AsstMsg msg, const json::value& details) const override;

protected:
    virtual bool _run() override;

private:
    RoguelikeStartSelect m_start_select; // 开局选择
    std::vector<std::string> get_select_list() const;
};
}
