#include "RecruitConfig.h"

#include <algorithm>

#include <meojson/json.hpp>

#include "Utils/Logger.hpp"

std::string asst::RecruitConfig::get_tag_name(const TagId& id) const noexcept
{
    auto iter = m_all_tags_name.find(id);
    if (iter == m_all_tags_name.cend()) {
        return "Unknown tag";
    }
    return iter->second;
}

std::string asst::RecruitConfig::find_tag_id(const std::string& name) const noexcept
{
    for (const auto& [id, tag_name] : m_all_tags_name) {
        if (tag_name == name) {
            return id;
        }
    }
    return "Unknown tag";
}

bool asst::RecruitConfig::parse(const json::value& json)
{
    LogTraceFunction;

    clear();

    for (const auto& [id, name] : json.at("tags").as_object()) {
        m_all_tags_name.emplace(id, name);
    }

    // tags的列表里有些tag可能是多余的，不出现在operators的tags里
    for (const json::value& oper : json.at("operators").as_array()) {
        Recruitment oper_temp;
        oper_temp.name = oper.at("name").as_string();
        oper_temp.id = oper.at("id").as_string();

        oper_temp.level = oper.at("rarity").as_integer();
        for (const json::value& tag_value : oper.at("tags").as_array()) {
            std::string tag = tag_value.as_string();
            oper_temp.tags.emplace(tag);
            m_all_tags_displayed.emplace(get_tag_name(tag));
            m_all_tags.emplace(std::move(tag));
        }
        m_all_opers.emplace_back(std::move(oper_temp));
    }

    // 按干员等级排个序
    ranges::sort(m_all_opers, std::greater {}, std::mem_fn(&Recruitment::level));

    return true;
}

void asst::RecruitConfig::clear()
{
    LogTraceFunction;

    m_all_opers.clear();
    m_all_tags.clear();
    m_all_tags_displayed.clear();
    m_all_tags_name.clear();
}
