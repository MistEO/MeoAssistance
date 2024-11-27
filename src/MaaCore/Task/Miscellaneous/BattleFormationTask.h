#pragma once

#include "Common/AsstBattleDef.h"
#include "OperList.h"
#include "Task/AbstractTask.h"
#include "Vision/TemplDetOCRer.h"

namespace asst
{
class BattleFormationTask : public AbstractTask
{
public:
    using AbstractTask::AbstractTask;
    virtual ~BattleFormationTask() override = default;

    using RequiredOper = battle::OperUsage;
    using RequiredOperGroups = battle::copilot::OperUsageGroups;
    using Role = battle::Role;

    // 设置追加自定干员列表
    void set_user_additional(std::vector<std::pair<std::string, int>> value) { m_user_additional = std::move(value); }

    bool set_squad_index(unsigned squad_index);

    std::shared_ptr<std::unordered_map<std::string, std::string>> get_opers_in_formation() const
    {
        return m_opers_in_formation;
    }

    enum class DataResource
    {
        Copilot,
        SSSCopilot,
    };

    void set_data_resource(const DataResource resource) { m_data_resource = resource; }

    // ————————————————————————————————————————————————————————————————————————————————
    // Supplementary Operator-Related
    // ————————————————————————————————————————————————————————————————————————————————
    using SortKey = OperList::SortKey;

    struct SupplementaryOperReq
    {
        Role role = OperList::ROLE_ALL;    // 筛选职业
        SortKey sort_key = SortKey::Level; // 排序键
        bool ascending = false;            // 是否采用递增
        unsigned num = 0;                  // 数量
    };

    void append_supplementary_oper_req(const SupplementaryOperReq req) { m_supplementary_oper_reqs.emplace_back(req); }

    // ————————————————————————————————————————————————————————————————————————————————
    // Support Unit-Related
    // ————————————————————————————————————————————————————————————————————————————————
    enum class SupportUnitUsage // 助战干员使用策略
    {
        None = 0,               // 不使用助战干员
                  // （以下都有）如果有且仅有一名缺失干员则尝试寻找助战干员补齐编队
        WhenNeeded = 1, // 如果无缺失干员则不使用助战干员
        Specific = 2,   // 如果无缺失干员则使用指定助战干员
        Random = 3      // 如果无缺失干员则使用随机助战干员
    };

    void set_support_unit_usage(const SupportUnitUsage support_unit_usgae)
    {
        m_support_unit_usage = support_unit_usgae;
    }

    bool set_specific_support_unit(const std::string& name = ""); // 设置指定助战干员

    bool add_support_unit(
        const std::vector<RequiredOper>& required_opers = {},
        int max_refresh_times = 0,
        bool max_spec_lvl = true,
        bool allow_non_friend_support_unit = false);

protected:
    virtual bool _run() override;

private:
    using OperGroup = std::pair<std::string, std::vector<asst::battle::OperUsage>>;

    bool add_formation(battle::Role role, std::vector<OperGroup> oper_group, std::vector<OperGroup>& missing);

    bool add_trust_operators(); // to be removed

    bool enter_selection_page();
    bool select_opers_in_cur_page(std::vector<OperGroup>& groups);
    void swipe_page();
    void swipe_to_the_left(int times = 2);
    bool confirm_selection();
    bool parse_formation();
    bool select_formation(int select_index);
    bool select_random_support_unit();
    void report_missing_operators(std::vector<OperGroup>& groups);

    std::vector<asst::TemplDetOCRer::Result> analyzer_opers();

    static constexpr unsigned NUM_SQUADS = 4; // 可用编队数量，亦为编队索引上限
    unsigned m_squad_index = 0; // 所使用的编队的索引，0 代表默认，1~NUM_SQUADS 分别代表第几编队

    DataResource m_data_resource = DataResource::Copilot;

    std::string m_stage_name;
    std::unordered_map<battle::Role, std::vector<OperGroup>> m_formation;
    std::unordered_map<battle::Role, std::vector<OperGroup>> m_user_formation;
    int m_size_of_operators_in_formation = 0;                             // 编队中干员个数
    std::shared_ptr<std::unordered_map<std::string, std::string>> m_opers_in_formation =
        std::make_shared<std::unordered_map<std::string, std::string>>(); // 编队中的干员名称-所属组名
    std::vector<std::pair<std::string, int>> m_user_additional;           // 追加干员表，从头往后加
    std::string m_last_oper_name;
    int m_missing_retry_times = 1;                                        // 识别不到干员的重试次数

    bool m_formation_is_full = false;                                     // 编队是否已满

    // ————————————————————————————————————————————————————————————————————————————————
    // Supplementary Operator-Related
    // ————————————————————————————————————————————————————————————————————————————————
    /// <summary>
    /// 在编队中添加附加干员。
    /// </summary>
    void add_supplementary_opers();

    std::vector<SupplementaryOperReq> m_supplementary_oper_reqs; // 附加干员需求列表

    // ————————————————————————————————————————————————————————————————
    // Support Unit Related
    // ————————————————————————————————————————————————————————————————
    static constexpr bool is_valid_support_unit_usage(const SupportUnitUsage support_unit_usage)
    {
        return support_unit_usage == SupportUnitUsage::None || support_unit_usage == SupportUnitUsage::WhenNeeded ||
               support_unit_usage == SupportUnitUsage::Specific || support_unit_usage == SupportUnitUsage::Random;
    }

    /// <summary>
    /// 在职业 role 的助战列表中寻找一名列于 required_opers 中的干员并使用其指定技能；
    /// 若 required_opers 为空则在当前职业的助战列表中随机选择一名干员并使用其默认技能。
    /// </summary>
    /// <param name="required_opers">
    /// 所需助战干员列表。
    /// </param>
    /// <param name="max_refresh_times">最大刷新助战列表的次数。</param>
    /// <param name="max_spec_lvl">是否要求技能专三。</param>
    /// <param name="allow_non_friend_support_unit">是否允许使用非好友助战干员。</param>
    /// <returns>
    /// 若成功找到并使用所需的助战干员，则返回 true，反之则返回 false。
    /// </returns>
    /// <remarks>
    /// 默认已经点开助战列表；
    /// 每次只能识别一页助战列表，因此最多会识别 MaxNumSupportListPages * (max_refresh_times + 1) 次；
    /// 若识别到多个满足条件的干员，则优先选择在 required_opers 中排序靠前的干员；
    /// 当 required_opers 为空时因没有指定技能，max_spec_lvl == true 仅会要求助战干员的专精等级达到 2。
    /// </remarks>
    bool add_support_unit_(
        const std::vector<RequiredOper>& required_opers = {},
        int max_refresh_times = 0,
        bool max_spec_lvl = true,
        bool allow_non_friend_support_unit = false);

    SupportUnitUsage m_support_unit_usage = SupportUnitUsage::None;
    RequiredOper m_specific_support_unit; // 仅当 m_support_unit_usage == SupportUnitUsage::Specific 时有效
    std::string m_used_support_unit_name; // 已招募的助战干员的名字
};
} // namespace asst
