#include "RecruitImageAnalyzer.h"

#include "Config/Miscellaneous/RecruitConfig.h"
#include "Config/TaskData.h"
#include "Utils/Logger.hpp"
#include "Vision/Matcher.h"
#include "Vision/MultiMatcher.h"
#include "Vision/OCRer.h"

bool asst::RecruitImageAnalyzer::analyze()
{
    m_tags_result.clear();

    bool ret0 = time_analyze();
    bool ret1 = refresh_analyze();
    bool ret2 = permit_analyze();
    bool ret3 = tags_analyze();

    Log.trace("time_analyze:", ret0, "refresh_analyze:", ret1, "permit_analyze:", ret2, "tags_analyze:", ret3);

    return ret0 && ret3;
}

bool asst::RecruitImageAnalyzer::tags_analyze()
{
    static bool analyzer_inited = false;
    static OCRer tags_analyzer;
    if (!analyzer_inited) {
        tags_analyzer.set_task_info("RecruitTags");
        auto& all_tags_set = RecruitData.get_all_tags_displayed();

        // 已经 fullMatch，不会再把 `支援机械` 匹配成 `支援`、`高级资深干员` 匹配成 `资深干员` 了，因此不必再排序。
        tags_analyzer.set_required(std::vector(all_tags_set.begin(), all_tags_set.end()));
        analyzer_inited = true;
    }

    tags_analyzer.set_image(m_image);

    if (tags_analyzer.analyze()) {
        std::vector<asst::TextRect> result = tags_analyzer.get_result();
        for (auto& tag : result) {
            tag.text = RecruitData.find_tag_id(tag.text);
        }

        m_tags_result = std::move(result);
        return true;
        // if (m_tags_result.size() == RecruitData.CorrectNumberOfTags) {
        //     return true;
        // }
    }
    return false;
}

bool asst::RecruitImageAnalyzer::time_analyze()
{
    MultiMatcher decrement_a(m_image);
    decrement_a.set_task_info("RecruitTimerDecrement");
    auto result_opt = decrement_a.analyze();
    if (!result_opt) {
        return false;
    }
    if (result_opt->size() != 2) {
        return false; // expecting two buttons
    }
    sort_by_horizontal_(*result_opt);
    m_hour_decrement = result_opt->at(0).rect;
    m_minute_decrement = result_opt->at(1).rect;
    return true;
}

bool asst::RecruitImageAnalyzer::refresh_analyze()
{
    Matcher refresh_analyzer(m_image);

    refresh_analyzer.set_task_info("RecruitRefresh");

    if (refresh_analyzer.analyze()) {
        m_refresh_rect = refresh_analyzer.get_result().rect;
        return true;
    }

    return false;
}

bool asst::RecruitImageAnalyzer::permit_analyze()
{
    Matcher permit_analyzer(m_image);

    permit_analyzer.set_task_info("RecruitNoPermit");

    if (permit_analyzer.analyze()) {
        m_permit_rect = permit_analyzer.get_result().rect;
        return true;
    }

    return false;
}
