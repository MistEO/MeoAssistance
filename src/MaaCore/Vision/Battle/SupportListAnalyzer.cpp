#include "SupportListAnalyzer.h"

#include "Common/AsstBattleDef.h"
#include "Config/TaskData.h"
#include "Utils/NoWarningCV.h"
#include "Vision/Matcher.h"
#include "Vision/MultiMatcher.h"
#include "Vision/RegionOCRer.h"

bool asst::SupportListAnalyzer::analyze(const battle::Role role)
{
    LogTraceFunction;

    MultiMatcher flag_analyzer(m_image);
    flag_analyzer.set_task_info("SupportListAnalyzer-Flag");
    if (!flag_analyzer.analyze()) {
        return false;
    }

    const auto& name_task_ptr = Task.get<OcrTaskInfo>("SupportListAnalyzer-NameOcr");
    const auto& elite_task_ptr = Task.get<MatchTaskInfo>("SupportListAnalyzer-Elite");
    const auto& level_task_ptr = Task.get<OcrTaskInfo>("SupportListAnalyzer-LevelOcr");

    RegionOCRer ocr_analyzer(m_image);
    Matcher elite_analyzer(m_image);

    std::vector<SupportUnit> results;
    for (const auto& [rect, score, templ_name] : flag_analyzer.get_result()) {
#ifdef ASST_DEBUG
        cv::rectangle(m_image_draw, make_rect<cv::Rect>(rect), cv::Scalar(0, 255, 0), 2);
#endif

        // 干员名称
        const Rect name_roi = rect.move(name_task_ptr->rect_move);
#ifdef ASST_DEBUG
        cv::rectangle(m_image_draw, make_rect<cv::Rect>(name_roi), cv::Scalar(0, 255, 0), 2);
#endif

        ocr_analyzer.set_task_info(name_task_ptr);
        ocr_analyzer.set_roi(name_roi);
        if (!ocr_analyzer.analyze()) {
            continue;
        }

        // 根据 role 标准化干员名以区分不同升变形态下的阿米娅
        const std::string name = standard_oper_name(role, ocr_analyzer.get_result().text);

        // 干员精英化等级
        const Rect elite_roi = rect.move(elite_task_ptr->rect_move);
#ifdef ASST_DEBUG
        cv::rectangle(m_image_draw, make_rect<cv::Rect>(elite_roi), cv::Scalar(0, 255, 0), 2);
#endif

        elite_analyzer.set_task_info(elite_task_ptr);
        elite_analyzer.set_roi(elite_roi);
        int elite = 0;
        if (elite_analyzer.analyze()) {
            elite = (elite_analyzer.get_result().templ_name == "SupportListAnalyzer-Elite-2" ? 2 : 1);
        }

        // 干员等级
        const Rect level_roi = rect.move(level_task_ptr->rect_move);
#ifdef ASST_DEBUG
        cv::rectangle(m_image_draw, make_rect<cv::Rect>(level_roi), cv::Scalar(0, 255, 0), 2);
#endif

        ocr_analyzer.set_task_info(level_task_ptr);
        ocr_analyzer.set_roi(level_roi);
        if (!ocr_analyzer.analyze()) {
            continue;
        }
        int level = 0;
        if (!utils::chars_to_number(ocr_analyzer.get_result().text, level)) {
            Log.error(__FUNCTION__, "| Fail to convert text", ocr_analyzer.get_result().text, "to number");
            save_img(utils::path("debug") / utils::path("supportListAnalyzer"));
            continue;
        }

        SupportUnit support_unit {
            .rect = rect,
            .name = name,
            .elite = elite,
            .level = level,
            .from_friend = (templ_name == "SupportListAnalyzer-Friend.png"),
        };
        LogInfo << __FUNCTION__ << "| Found support operator from"
                << (support_unit.from_friend ? "friend:" : "non-friend:") << support_unit.name << "with elite"
                << support_unit.elite << "and level" << support_unit.level;
        results.emplace_back(std::move(support_unit));
    }
    sort_by_horizontal_(results);

    m_result = std::move(results);
    return !m_result.empty();
}
