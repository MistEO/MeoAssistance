#include "RoguelikeCollapsalParadigmTaskPlugin.h"

#include <algorithm>

#include "Config/Roguelike/Sami/RoguelikeCollapsalParadigmConfig.h"
#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Matcher.h"
#include "Vision/OCRer.h"

bool asst::RoguelikeCollapsalParadigmTaskPlugin::load_params(const json::value& params)
{
    // 检查适用主题, 仅萨米肉鸽使用
    const std::string& theme = m_config->get_theme();
    if (theme != RoguelikeTheme::Sami) {
        return false;
    }

    // 根据 params 设置插件专用参数
    const RoguelikeMode mode = m_config->get_mode();
    if (!params.get("check_collapsal_paradigms", mode == RoguelikeMode::CLP_PDS)) {
        return false;
    }
    m_double_check_clp_pds = params.get("double_check_collapsal_paradigms", mode == RoguelikeMode::CLP_PDS);

    m_expected_clp_pds =
        params.get("expected_collapsal_paradigms", RoguelikeCollapsalParadigms.get_rare_clp_pds(theme));
    Log.info(__FUNCTION__, "| Expected collapsal paradigms are", m_expected_clp_pds);

    // 从 tasks.json 获取插件设置，由于仅有萨米肉鸽使用，任务名暂定写死
    const auto& bannerCheckConfig = Task.get<OcrTaskInfo>(theme + "@Roguelike@CollapsalParadigmTaskBannerCheckConfig");
    const auto& panelCheckConfig = Task.get<OcrTaskInfo>(theme + "@Roguelike@CollapsalParadigmTaskPanelCheckConfig");

    m_deepen_text = bannerCheckConfig->text.front();

    m_banner_triggers_start = std::unordered_set(bannerCheckConfig->sub.begin(), bannerCheckConfig->sub.end());
    m_banner_triggers_completed = std::unordered_set(bannerCheckConfig->next.begin(), bannerCheckConfig->next.end());
    m_panel_triggers = std::unordered_set(panelCheckConfig->sub.begin(), panelCheckConfig->sub.end());
    m_zone_dict = std::unordered_map(panelCheckConfig->replace_map.begin(), panelCheckConfig->replace_map.end());

    return true;
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::reset_in_run_variables()
{
    m_clp_pds.clear();
    m_check_banner = false;
    m_check_panel = false;
    m_need_check_panel = false;
    m_verification_check = false;
    m_zone.clear();
}

bool asst::RoguelikeCollapsalParadigmTaskPlugin::verify(const AsstMsg msg, const json::value& details) const
{
    if ((msg != AsstMsg::SubTaskStart && msg != AsstMsg::SubTaskCompleted) ||
        details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    const auto task_name = details.get("details", "task", "");

    // banner check
    if (msg == AsstMsg::SubTaskStart && m_banner_triggers_start.contains(task_name)) {
        m_check_banner = true;
        return true;
    }
    if (msg == AsstMsg::SubTaskCompleted && m_banner_triggers_completed.contains(task_name)) {
        m_check_banner = true;
        return true;
    }

    // panel check
    if (msg == AsstMsg::SubTaskStart && m_panel_triggers.contains(task_name)) {
        if (new_zone()) {
            m_need_check_panel = true;
            m_verification_check = false;
        }

        if (m_need_check_panel) {
            m_need_check_panel = false;
            m_check_panel = true;
            return true;
        }
    }

    if (msg == AsstMsg::SubTaskCompleted && !m_panel_triggers.contains(task_name) && !m_need_check_panel &&
        m_double_check_clp_pds) {
        m_need_check_panel = true;
        m_verification_check = true;
    };

    return false;
}

bool asst::RoguelikeCollapsalParadigmTaskPlugin::_run()
{
    if (m_check_banner) {
        m_check_banner = false;
        check_banner();
    }
    if (m_check_panel) {
        m_check_panel = false;
        check_panel();
    }
    return true;
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::check_banner()
{
    LogTraceFunction;

    const std::string& theme = m_config->get_theme();

    // analyzer1 检测坍缩范式变动，即 "坍缩范式加深" 或 "坍缩范式消退"
    wait_for_loading();
    cv::Mat image = ctrler()->get_image();
    OCRer analyzer1(image);
    analyzer1.set_task_info(theme + "@Roguelike@CheckCollapsalParadigmsOnBanner");
    auto detected = analyzer1.analyze();
    if (!detected) { // 以防万一，等一等再识别一次
        wait_for_loading();
        image = ctrler()->get_image();
        analyzer1.set_image(image);
        detected = analyzer1.analyze();
    }
    if (detected) {
        const std::vector<CollapsalParadigmClass>& clp_pd_classes =
            RoguelikeCollapsalParadigms.get_clp_pd_classes(theme);
        const std::unordered_map<std::string, unsigned int>& clp_pd_dict =
            RoguelikeCollapsalParadigms.get_clp_pd_dict(theme);
        std::vector<std::string> prev_clp_pds = m_clp_pds;

        OCRer analyzer2(image); // 检测坍缩范式名
        analyzer2.set_task_info(theme + "@Roguelike@CheckCollapsalParadigmsBanner");
        if (!analyzer2.analyze()) {
            Log.info(m_banner_check_error_message);
            return;
        }

        OCRer::ResultsVec ocr_results1 = analyzer1.get_result();
        OCRer::ResultsVec ocr_results2 = analyzer2.get_result();
        sort_by_vertical_(ocr_results1); // 按照垂直方向排序（从上到下）
        sort_by_vertical_(ocr_results2); // 按照垂直方向排序（从上到下）

        auto result_it1 = ocr_results1.begin();
        auto result_it2 = ocr_results2.begin();

        std::vector<std::string>::iterator it;
        while (result_it1 != ocr_results1.end() && result_it2 != ocr_results2.end()) {
            // 如果坍缩范式名与坍缩范式变动之间的距离太远，则提示识别出错，跳到下一个坍缩范式变动
            if (result_it2->rect.y >= result_it1->rect.y + result_it1->rect.height + 25) {
                Log.info(m_banner_check_error_message);
                ++result_it1;
                continue;
            }
            // 如果坍缩范式名在坍缩范式变动之前，则提示识别出错，跳到下一个坍缩范式名
            else if (result_it2->rect.y <= result_it1->rect.y) {
                Log.info(m_banner_check_error_message);
                ++result_it2;
                continue;
            }

            std::string cur_clp_pd = result_it2->text;
            const CollapsalParadigmClass& cur_class = clp_pd_classes.at(clp_pd_dict.at(cur_clp_pd));

            // 如果检测到坍缩范式加深，根据之前的坍缩范式，将其添加或替换到当前拥有的坍缩范式列表
            if (result_it1->text == m_deepen_text) {
                // 已拥有坍缩范式，跳到下一组
                if (it = std::ranges::find(prev_clp_pds, cur_clp_pd); it != prev_clp_pds.end()) {
                    ++result_it1;
                    ++result_it2;
                    continue;
                }
                // 新获得二级坍缩范式，检查是否拥有其一级，如果获得是已拥有的一级坍缩范式，会在上一个 if 进行判定
                if (it = std::ranges::find(prev_clp_pds, cur_class.level_1); it != prev_clp_pds.end()) {
                    clp_pd_callback(cur_clp_pd, 1, *it);
                    *it = cur_clp_pd;
                }
                else {
                    clp_pd_callback(cur_clp_pd, 1);
                    prev_clp_pds.emplace_back(cur_clp_pd);
                }

                // 如果 cur_clp_pd 是需要的坍缩范式
                if (m_expected_clp_pds.contains(cur_clp_pd)) {
                    exit_then_stop();
                    return;
                }
                // 刷坍缩范式模式下，获得了不想要的坍缩范式
                else if (m_config->get_mode() == RoguelikeMode::CLP_PDS) {
                    exit_then_restart();
                    return;
                }
            }
            // 如果检测到坍缩范式消退，根据之前的坍缩范式，将其移除当前拥有的坍缩范式列表或替换为低一级的坍缩范式
            else {
                // 未拥有坍缩范式，跳到下一组
                if (it = std::ranges::find(prev_clp_pds, cur_clp_pd); it == prev_clp_pds.end()) {
                    ++result_it1;
                    ++result_it2;
                    continue;
                }
                if (cur_clp_pd == cur_class.level_2) {
                    clp_pd_callback(cur_class.level_1, -1, cur_clp_pd);
                    *it = cur_class.level_1;
                    // 如果2级直接消退到0级的话会有几次提示？这里可能会出问题，谨慎一些，晚些再检查一次。
                    m_need_check_panel = true;
                }
                else {
                    clp_pd_callback("", -1, cur_clp_pd);
                    prev_clp_pds.erase(it);
                }
            }
            ++result_it1;
            ++result_it2;
        }
        m_clp_pds = std::move(prev_clp_pds);
    }
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::check_panel()
{
    LogTraceFunction;

#ifdef ASST_DEBUG
    if (m_verification_check) {
        Log.info(__FUNCTION__, "| Verification");
    }
#endif

    const std::string& theme = m_config->get_theme();
    const std::vector<CollapsalParadigmClass>& clp_pd_classes = RoguelikeCollapsalParadigms.get_clp_pd_classes(theme);
    const std::unordered_map<std::string, unsigned int>& clp_pd_dict =
        RoguelikeCollapsalParadigms.get_clp_pd_dict(theme);
    std::vector<std::string> prev_clp_pds = m_clp_pds;

    wait_for_stage();

    cv::Mat image;
    OCRer analyzer;
    analyzer.set_task_info(theme + "@Roguelike@CheckCollapsalParadigmsOnPanel");
    do {
        toggle_panel();
        image = ctrler()->get_image();
        analyzer.set_image(image);
    } while (!need_exit() && !analyzer.analyze()); // 检测 "当前坍缩值" 字样以确保坍缩范式列表已展开

    // ================================================================================
    std::vector<std::string> cur_clp_pds;
    analyzer.set_task_info(theme + "@Roguelike@CheckCollapsalParadigmsPanel"); // 检测坍缩范式状态栏上的坍缩范式
    if (analyzer.analyze()) {
        OCRer::ResultsVec ocr_results = analyzer.get_result();
        // 识别到两个及以上坍缩范式的时候，向上滑动一下再识别一次
        // if detect more than two collapsal paradigms, swipe up and analyze again
        if (ocr_results.size() >= 2) {
            ProcessTask(*this, { theme + "@Roguelike@SwipeCollapsalParadigmPanelUp" }).run();
            analyzer.set_image(ctrler()->get_image());
            if (analyzer.analyze()) {
                ocr_results = analyzer.get_result();
            }
        }

        // 将 OCRer::Result 按照垂直方向排序（从上到下）并转换为坍缩范式名称
        sort_by_vertical_(ocr_results);
        std::ranges::transform(ocr_results, std::back_inserter(cur_clp_pds), [](const OCRer::Result& x) {
            return x.text;
        });
    }

    // ================================================================================
    toggle_panel(); // 坍缩范式状态栏检查完毕，关闭状态栏，以下为处理识别到的结果

    if (prev_clp_pds == cur_clp_pds) { // 如果坍缩范式没有变动
        return;
    }
    else if (m_verification_check) {
        Log.info(__FUNCTION__, "| Verification Failed");
#ifdef ASST_DEBUG
        Log.info("–––––––– Previous ––––––––––––––");
        for (const std::string& clp_pd : prev_clp_pds) {
            Log.info(clp_pd);
        }
        Log.info("–––––––– Current –––––––––––––––");
        for (const std::string& clp_pd : cur_clp_pds) {
            Log.info(clp_pd);
        }
#endif
    }

    auto it = prev_clp_pds.begin();
    for (const std::string& cur_clp_pd : cur_clp_pds) {
        // 判断坍缩范式变动
        const unsigned int& cur_index = clp_pd_dict.at(cur_clp_pd);
        const CollapsalParadigmClass& cur_class = clp_pd_classes.at(cur_index);
        while (it != prev_clp_pds.end() && clp_pd_dict.at(*it) != cur_index) {
            clp_pd_callback("", -1, *it);
            ++it;
        }
        if (it == prev_clp_pds.end()) { // 全新的坍缩范式
            clp_pd_callback(cur_clp_pd, 1);
        }
        else {
            if (cur_clp_pd == cur_class.level_2 && *it == cur_class.level_1) { // 从1级加深到2级
                clp_pd_callback(cur_clp_pd, 1, cur_class.level_1);
            }
            else if (cur_clp_pd == cur_class.level_1 && *it == cur_class.level_2) { // 从2级消退到1级
                clp_pd_callback(cur_clp_pd, -1, cur_class.level_2);
            }
            ++it;
        }
        // 判断 cur_clp_pd 是否为需要的坍缩范式
        if (m_expected_clp_pds.contains(cur_clp_pd)) {
            exit_then_stop();
            return;
        }
        // 刷坍缩范式模式下，获得了不想要的坍缩范式
        else if (m_config->get_mode() == RoguelikeMode::CLP_PDS) {
            exit_then_restart();
            return;
        }
    }

    // 之前拥有的坍缩范式没有被识别到，判定为消退掉了
    while (it != prev_clp_pds.end()) {
        clp_pd_callback("", -1, *it);
        ++it;
    }

    m_clp_pds = std::move(cur_clp_pds);
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::toggle_panel() const
{
    ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ToggleCollapsalParadigmPanel" }).run();
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::wait_for_loading() const
{
    OCRer analyzer(ctrler()->get_image());
    analyzer.set_task_info("LoadingText");
    while (!need_exit() && analyzer.analyze()) {
        sleep(100);
        analyzer.set_image(ctrler()->get_image());
    }
    sleep(200);
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::wait_for_stage() const
{
    Matcher matcher(ctrler()->get_image());
    matcher.set_task_info(m_config->get_theme() + "@Roguelike@CheckCollapsalParadigmsOnStage");
    while (!need_exit() && !matcher.analyze()) {
        sleep(100);
        matcher.set_image(ctrler()->get_image());
    }
    sleep(200);
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::exit_then_restart() const
{
    m_control_ptr->exit_then_stop();
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::exit_then_stop() const
{
    m_control_ptr->exit_then_stop();
    m_task_ptr->set_enable(false);
}

bool asst::RoguelikeCollapsalParadigmTaskPlugin::new_zone() const
{
    Matcher matcher(ctrler()->get_image());
    matcher.set_task_info(m_config->get_theme() + "@Roguelike@CheckCollapsalParadigmsOnStage");
    if (!matcher.analyze()) {
        return false;
    }
    std::string zone = m_zone_dict.at(matcher.get_result().templ_name);
    if (zone != m_zone) {
        m_zone = zone;
#ifdef ASST_DEBUG
        Log.info(__FUNCTION__, "Current Zone is " + m_zone);
#endif
        return true;
    }
    return false;
}

void asst::RoguelikeCollapsalParadigmTaskPlugin::clp_pd_callback(
    const std::string& cur,
    const int& deepen_or_weaken,
    const std::string& prev)
{
    json::value info = basic_info_with_what("RoguelikeCollapsalParadigms");
    info["details"]["cur"] = cur;
    info["details"]["deepen_or_weaken"] = deepen_or_weaken;
    info["details"]["prev"] = prev;
    callback(AsstMsg::SubTaskExtraInfo, info);
}
