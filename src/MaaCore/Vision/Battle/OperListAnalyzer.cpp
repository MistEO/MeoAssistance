#include "OperListAnalyzer.h"

#include "Common/AsstBattleDef.h"
#include "Config/TaskData.h"
#include "Utils/NoWarningCV.h"
#include "Vision/Matcher.h"
#include "Vision/MultiMatcher.h"
#include "Vision/RegionOCRer.h"

bool asst::OperListAnalyzer::analyze(const bool minimal)
{
    LogTraceFunction;

    MultiMatcher flag_analyzer(m_image);
    flag_analyzer.set_task_info("OperListAnalyzer-Flag");
    if (!flag_analyzer.analyze()) {
        return false;
    }

    const auto& selected_task_ptr = Task.get<MatchTaskInfo>("OperListAnalyzer-Selected");
    const auto& name_task_ptr = Task.get<OcrTaskInfo>("OperListAnalyzer-NameOcr");

    Matcher selected_analyzer(m_image);
    RegionOCRer ocr_analyzer(m_image);

    std::vector<CandidateOper> results;
    for (const auto& [rect, score, templ_name] : flag_analyzer.get_result()) {
#ifdef ASST_DEBUG
        cv::rectangle(m_image_draw, make_rect<cv::Rect>(rect), cv::Scalar(0, 255, 0), 2);
#endif

        CandidateOper candidate_oper { .rect = rect };

        // 干员选择状态
        const Rect selected_roi = rect.move(selected_task_ptr->rect_move);
#ifdef ASST_DEBUG
        cv::rectangle(m_image_draw, make_rect<cv::Rect>(selected_roi), cv::Scalar(0, 255, 0), 2);
#endif

        selected_analyzer.set_task_info(selected_task_ptr);
        selected_analyzer.set_roi(selected_roi);
        candidate_oper.selected = selected_analyzer.analyze().has_value();

        // 干员职业
        const std::string role_str = templ_name.substr(0, templ_name.find('@'));
        candidate_oper.role = battle::get_role_type(role_str);

        // placeHolder: 干员精英化等级也可以放在这里识别

        if (!minimal) {
            // 干员名称
            const Rect name_roi = rect.move(name_task_ptr->rect_move);
#ifdef ASST_DEBUG
            cv::rectangle(m_image_draw, make_rect<cv::Rect>(name_roi), cv::Scalar(0, 255, 0), 2);
#endif

            ocr_analyzer.set_task_info(name_task_ptr);
            ocr_analyzer.set_roi(name_roi);
            ocr_analyzer.set_bin_threshold(name_task_ptr->special_params[0]);
            ocr_analyzer.set_bin_expansion(name_task_ptr->special_params[1]);
            ocr_analyzer.set_bin_trim_threshold(name_task_ptr->special_params[2], name_task_ptr->special_params[3]);
            if (!ocr_analyzer.analyze()) {
                continue;
            }

            // 根据 role 标准化干员名以区分不同升变形态下的阿米娅
            candidate_oper.name = standard_oper_name(candidate_oper.role, ocr_analyzer.get_result().text);
        }

        LogInfo << __FUNCTION__ << "| Found" << (candidate_oper.selected ? "selected" : "unselected")
                << "operator:" << candidate_oper.name << "with role" << enum_to_string(candidate_oper.role);
        results.emplace_back(std::move(candidate_oper));
    }
    sort_by_vertical_(results);

    m_result = std::move(results);
    return !m_result.empty();
}
