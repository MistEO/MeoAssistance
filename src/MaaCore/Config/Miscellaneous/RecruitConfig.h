#pragma once

#include "Config/AbstractConfig.h"

#include "Utils/Ranges.hpp"
#include <algorithm>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

#include "Common/AsstTypes.h"

namespace asst
{
// 干员信息，公开招募相关
struct Recruitment
{
    std::string name;
    std::string id;
    int level = 0;
    std::unordered_set<std::string> tags;

    bool has_tag(const std::string& tag) const { return tags.contains(tag); }

    friend std::strong_ordering operator<=>(const Recruitment& lhs, const Recruitment& rhs)
    {
        if (lhs.level != rhs.level) {
            return lhs.level <=> rhs.level; // increment order
        }
#ifdef __clang__
        if (rhs.name < lhs.name) {
            return std::strong_ordering::less;
        }
        else if (rhs.name > lhs.name) {
            return std::strong_ordering::greater;
        }
        else {
            return std::strong_ordering::equal;
        }
#else
        return rhs.name <=> lhs.name; // decrement order (print reversely)
#endif
    }

    friend bool operator==(const Recruitment& lhs, const Recruitment& rhs)
    {
        return lhs.id == rhs.id && lhs.level == rhs.level;
    }
};

// 选择额外 Tags 的模式
enum class ExtraTagsMode
{
    NoExtra,       // 0 - 默认行为
    Extra,         // 1 - 选 3 个 Tags, 即使可能冲突
    ExtraOnlyRare, // 2 - 如果可能, 同时选择更多的高星 Tag 组合, 即使可能冲突
};

class RecruitConfig final : public SingletonHolder<RecruitConfig>, public AbstractConfig
{
public:
    using TagId = std::string;

public:
    static constexpr bool is_valid_extra_tags_mode(ExtraTagsMode mode)
    {
        return mode == ExtraTagsMode::NoExtra || mode == ExtraTagsMode::Extra || mode == ExtraTagsMode::ExtraOnlyRare;
    }

public:
    virtual ~RecruitConfig() override = default;
    static constexpr int CorrectNumberOfTags = 5;

    const std::unordered_set<std::string>& get_all_tags() const noexcept { return m_all_tags; }

    const std::vector<Recruitment>& get_all_opers() const noexcept { return m_all_opers; }

    std::string get_tag_name(const TagId& id) const noexcept;
    // 根据 外文/中文tag 名称查找 中文tag id
    std::string find_tag_id(const std::string& name) const noexcept;

protected:
    virtual bool parse(const json::value& json) override;

    void clear();

    std::unordered_set<std::string> m_all_tags;
    std::vector<Recruitment> m_all_opers;
    std::unordered_map<TagId, std::string> m_all_tags_name; // <中文tag, 中文/外文tag>
};

// 公开招募的干员组合
struct RecruitCombs
{
    // TODO: using vector here can be expensive
    std::vector<RecruitConfig::TagId> tags;
    std::vector<Recruitment> opers;
    int max_level = 0;
    int min_level = 0;
    double avg_level = 0;

    void update_attributes()
    {
        min_level = std::transform_reduce(
            opers.cbegin(),
            opers.cend(),
            7,
            [](int a, int b) -> int { return (std::min)(a, b); },
            std::mem_fn(&Recruitment::level));

        max_level = std::transform_reduce(
            opers.cbegin(),
            opers.cend(),
            0,
            [](int a, int b) -> int { return (std::max)(a, b); },
            std::mem_fn(&Recruitment::level));

        avg_level = std::transform_reduce(
                        opers.cbegin(),
                        opers.cend(),
                        0.,
                        std::plus<double> {},
                        std::mem_fn(&Recruitment::level)) /
                    static_cast<double>(opers.size());
    }

    // intersection of two recruit combs
    friend RecruitCombs operator*(RecruitCombs& lhs, RecruitCombs& rhs)
    {
        ranges::sort(lhs.tags);
        ranges::sort(lhs.opers);
        ranges::sort(rhs.tags);
        ranges::sort(rhs.opers);

        RecruitCombs result;

        ranges::set_union(lhs.tags, rhs.tags, std::back_inserter(result.tags));

        ranges::set_intersection(lhs.opers, rhs.opers, std::back_inserter(result.opers));

        result.update_attributes();

        return result;
    }
};

inline static auto& RecruitData = RecruitConfig::get_instance();
} // namespace asst
