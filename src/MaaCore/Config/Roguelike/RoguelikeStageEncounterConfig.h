#pragma once

#include "Config/AbstractConfig.h"

#include <vector>

#include "Common/AsstBattleDef.h"
#include "Task/Roguelike/RoguelikeConfig.h"

namespace asst
{
// steal from https://www.boost.org/doc/libs/1_85_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
// use boost if you prefer
template <typename T1, typename T2>
struct PairHash
{
    std::size_t operator()(const std::pair<T1, T2>& p) const
    {
        std::size_t hash1 = std::hash<T1> {}(p.first);
        std::size_t hash2 = std::hash<T2> {}(p.second);
        hash1 ^= hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2);

        hash1 ^= hash1 >> 32;
        hash1 *= 0xe9846af9b1a615d;
        hash1 ^= hash1 >> 32;
        hash1 *= 0xe9846af9b1a615d;
        hash1 ^= hash1 >> 28;

        return hash1;
    }
};

class RoguelikeStageEncounterConfig final : public SingletonHolder<RoguelikeStageEncounterConfig>, public AbstractConfig
{
public:
    virtual ~RoguelikeStageEncounterConfig() override = default;

    const auto& get_events(const std::string& theme, const RoguelikeMode mode) const noexcept
    {
        std::pair<std::string, int> key = std::make_pair(theme, static_cast<int>(mode));
        if (!m_events.contains(key)) {
            key.second = -1;
        }
        return m_events.at(key);
    }

    const auto& get_event_names(const std::string& theme) const noexcept { return m_event_names.at(theme); }

    bool set_event(
        const std::string& theme,
        RoguelikeMode mode,
        const std::string& event_name,
        int choose,
        int option_num);

    enum class ComparisonType
    {
        GreaterThan,
        LessThan,
        Equal,
        None,        // 没有配置
        Unsupported, // 配置错误或其他
    };

    struct Vision
    {
        std::string value;
        ComparisonType type = ComparisonType::None;
    };

    struct ChoiceRequire
    {
        std::string name;
        int choose = -1;
        Vision vision; // 现在只有Vision解析，之后要改成requirements且支持多个条件判断同一个选择
    };

    struct RoguelikeEvent
    {
        std::string name;
        int option_num = 0;
        int default_choose = 0;
        std::vector<ChoiceRequire> choice_require;
    };

private:
    virtual bool parse(const json::value& json) override;

    static ComparisonType parse_comparison_type(const std::string& type_str);

    std::unordered_map<
        std::pair<std::string, int>,
        std::unordered_map<std::string, RoguelikeEvent>,
        PairHash<std::string, int>>
        m_events;
    std::unordered_map<std::string, std::vector<std::string>> m_event_names;

    std::unordered_map<
        std::pair<std::string, int>,
        std::unordered_map<std::string, RoguelikeEvent>,
        PairHash<std::string, int>>
        m_events_temp;

    // 未指定适用模式（即 mode 字段为空）的配置将作为默认配置，以 "(theme, DEFAULT_MODE_PLACEHOLDER)" 为 key
    // 对于具体模式 mode，其专用配置，以 (theme, mode) 为 key，将继承默认配置
    static constexpr int DEFAULT_MODE_PLACEHOLDER = -1;
};

inline static auto& RoguelikeStageEncounter = RoguelikeStageEncounterConfig::get_instance();
}
