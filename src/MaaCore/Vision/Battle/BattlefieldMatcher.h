#pragma once
#include "Vision/VisionHelper.h"

#include "Common/AsstBattleDef.h"

namespace asst
{
class BattlefieldMatcher : public VisionHelper
{
public:
    struct ObjectOfInterest
    {
        bool flag = true;
        bool deployment = false;
        bool kills = false;
        bool costs = false;
        bool cost_regeneration = false;
        // bool in_detail = false;
        bool speed_button = false;
        bool oper_cost = false;
    };

    struct Result
    {
        ObjectOfInterest object_of_interest;

        std::vector<battle::DeploymentOper> deployment;
        // kills / total_kills
        std::optional<std::pair<int, int>> kills;
        std::optional<int> costs;
        std::optional<double> cost_regeneration;

        // bool in_detail = false;
        bool speed_button = false;
        bool pause_button = false;
    };

    using ResultOpt = std::optional<Result>;

public:
    using VisionHelper::VisionHelper;
    virtual ~BattlefieldMatcher() override = default;

    void set_object_of_interest(ObjectOfInterest obj);
    void set_total_kills_prompt(int prompt);

    ResultOpt analyze() const;

protected:
    bool hp_flag_analyze() const;
    bool kills_flag_analyze() const;
    bool pause_button_analyze() const;

    std::vector<battle::DeploymentOper> deployment_analyze() const; // 识别干员
    battle::Role oper_role_analyze(const Rect& roi) const;
    bool oper_cooling_analyze(const Rect& roi) const;
    int oper_cost_analyze(const Rect& roi) const;
    bool oper_available_analyze(const Rect& roi) const;

    std::optional<std::pair<int, int>> kills_analyze() const; // 识别击杀数
    bool cost_symbol_analyze() const;                         // 识别费用左侧图标
    std::optional<int> costs_analyze() const;                 // 识别费用
    std::optional<double> cost_regeneration_analyze() const;  // 识别费用回复进度条
    bool in_detail_analyze() const;                           // 识别是否在详情页
    bool speed_button_analyze() const;                        // 识别是否有加速按钮（在详情页就没有）

    ObjectOfInterest m_object_of_interest;                    // 待识别的目标
    int m_total_kills_prompt = 0; // 之前的击杀总数，因为击杀数经常识别不准所以依赖外部传入作为参考
};
} // namespace asst
