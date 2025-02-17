#include "BattlefieldMatcher.h"

#include "Utils/Ranges.hpp"
#include <algorithm>

#include "Utils/NoWarningCV.h"

#include "Config/TaskData.h"
#include "Config/TemplResource.h"
#include "Utils/Logger.hpp"
#include "Vision/BestMatcher.h"
#include "Vision/Matcher.h"
#include "Vision/Miscellaneous/BrightPointAnalyzer.h"
#include "Vision/MultiMatcher.h"
#include "Vision/RegionOCRer.h"
#include "Vision/TemplDetOCRer.h"

using namespace asst;

void BattlefieldMatcher::set_object_of_interest(ObjectOfInterest obj)
{
    m_object_of_interest = std::move(obj);
}

void BattlefieldMatcher::set_total_kills_prompt(int prompt)
{
    m_total_kills_prompt = prompt;
}

BattlefieldMatcher::ResultOpt BattlefieldMatcher::analyze() const
{
    Result result;

    if (m_object_of_interest.flag) {
        result.pause_button = pause_button_analyze();
        if (!result.pause_button && !hp_flag_analyze() && !kills_flag_analyze() && !cost_symbol_analyze()) {
            // flag 表明当前画面是在战斗场景的，不在的就没必要识别了
            return std::nullopt;
        }
    }

    if (m_object_of_interest.deployment) {
        result.deployment = deployment_analyze();
    }

    if (m_object_of_interest.kills) {
        result.kills = kills_analyze();
        if (!result.kills) {
            return std::nullopt;
        }
    }

    if (m_object_of_interest.costs) {
        result.costs = costs_analyze();
        if (!result.costs) {
            return std::nullopt;
        }
    }

    if (m_object_of_interest.cost_regeneration) {
        result.cost_regeneration = cost_regeneration_analyze();
        if (!result.cost_regeneration) {
            return std::nullopt;
        }
    }

    // 识别的不准，暂时不用了
    // if (m_object_of_interest.in_detail) {
    //    result.in_detail = in_detail_analyze();
    //}

    if (m_object_of_interest.speed_button) {
        result.speed_button = speed_button_analyze();
    }

    return result;
}

std::vector<battle::DeploymentOper> BattlefieldMatcher::deployment_analyze() const
{
    MultiMatcher flags_analyzer(m_image);
    const auto& flag_task_ptr = Task.get("BattleOpersFlag");
    flags_analyzer.set_task_info(flag_task_ptr);

#ifndef ASST_DEBUG
    flags_analyzer.set_log_tracing(false);
#endif

    auto flag_opt = flags_analyzer.analyze();
    if (!flag_opt) {
        return {};
    }
    auto& flags = flag_opt.value();
    sort_by_horizontal_(flags);

    const Rect& click_move = Task.get("BattleOperClickRange")->rect_move;
    const Rect& role_move = Task.get("BattleOperRoleRange")->rect_move;
    const Rect& avlb_move = Task.get("BattleOperAvailable")->rect_move;
    const Rect& cooling_move = Task.get("BattleOperCooling")->rect_move;
    const Rect& avatar_move = Task.get("BattleOperAvatar")->rect_move;
    const Rect& cost_move = Task.get("BattleOperCost")->rect_move;

    std::vector<battle::DeploymentOper> oper_result;
    size_t index = 0;
    for (const auto& flag_res : flags) {
        battle::DeploymentOper oper;
        oper.rect = flag_res.rect.move(click_move);

        Rect role_rect = flag_res.rect.move(role_move);
        oper.role = oper_role_analyze(role_rect);
        if (oper.role == battle::Role::Unknown) {
            Log.warn("Unknown role");
            continue;
        }

        if (oper.rect.x + oper.rect.width >= m_image.cols) {
            oper.rect.width = m_image.cols - oper.rect.x;
        }
        Rect avatar_rect = oper.rect.move(avatar_move);
        oper.avatar = m_image(make_rect<cv::Rect>(avatar_rect));

        Rect available_rect = flag_res.rect.move(avlb_move);
        oper.available = oper_available_analyze(available_rect);

#ifdef ASST_DEBUG
        if (oper.available) {
            cv::rectangle(m_image_draw, make_rect<cv::Rect>(oper.rect), cv::Scalar(0, 255, 0), 2);
        }
        else {
            cv::rectangle(m_image_draw, make_rect<cv::Rect>(oper.rect), cv::Scalar(0, 0, 255), 2);
        }
#endif

        Rect cooling_rect = correct_rect(flag_res.rect.move(cooling_move), m_image);
        oper.cooling = oper_cooling_analyze(cooling_rect);
        if (oper.cooling && oper.available) {
            Log.error("oper is available, but with cooling");
        }

#ifdef ASST_DEBUG
        if (oper.cooling) {
            cv::putText(
                m_image_draw,
                "cooling",
                cv::Point(oper.rect.x, oper.rect.y - 20),
                1,
                1.2,
                cv::Scalar(0, 0, 255));
        }
#endif

        if (m_object_of_interest.oper_cost) {
            Rect cost_rect = correct_rect(flag_res.rect.move(cost_move), m_image);
            oper.cost = oper_cost_analyze(cost_rect);
        }

        oper.index = index++;

        oper_result.emplace_back(std::move(oper));
    }

    return oper_result;
}

battle::Role BattlefieldMatcher::oper_role_analyze(const Rect& roi) const
{
    static const std::unordered_map<std::string, battle::Role> RoleMap = {
        { "Caster", battle::Role::Caster }, { "Medic", battle::Role::Medic },     { "Pioneer", battle::Role::Pioneer },
        { "Sniper", battle::Role::Sniper }, { "Special", battle::Role::Special }, { "Support", battle::Role::Support },
        { "Tank", battle::Role::Tank },     { "Warrior", battle::Role::Warrior }, { "Drone", battle::Role::Drone },
    };

    static const std::string TaskName = "BattleOperRole";
    static const std::string Ext = ".png";
    BestMatcher role_analyzer(m_image);
#ifndef ASST_DEBUG
    role_analyzer.set_log_tracing(false);
#endif // !ASST_DEBUG
    role_analyzer.set_task_info(TaskName);
    role_analyzer.set_roi(roi);

    for (const auto& role_name : RoleMap | views::keys) {
        role_analyzer.append_templ(TaskName + role_name + Ext);
    }
    auto role_opt = role_analyzer.analyze();
    if (!role_opt) {
        Log.warn(__FUNCTION__, "unknown role");
        return battle::Role::Unknown;
    }

    const auto& templ_name = role_opt->templ_info.name;

    std::string role_name = templ_name.substr(TaskName.size(), templ_name.size() - TaskName.size() - Ext.size());

#ifdef ASST_DEBUG
    cv::putText(m_image_draw, role_name, cv::Point(roi.x, roi.y - 5), 1, 1, cv::Scalar(0, 255, 255));
#endif

    return RoleMap.at(role_name);
}

bool BattlefieldMatcher::oper_cooling_analyze(const Rect& roi) const
{
    const auto cooling_task_ptr = Task.get<MatchTaskInfo>("BattleOperCooling");

    if (cooling_task_ptr->color_scales.size() != 1 ||
        !std::holds_alternative<MatchTaskInfo::ColorRange>(cooling_task_ptr->color_scales.front())) {
        Log.error(__FUNCTION__, "| color_scales in `BattleOperCooling` is not a ColorRange");
        return false;
    }

    const auto& color_scale = std::get<MatchTaskInfo::ColorRange>(cooling_task_ptr->color_scales.front());
    auto img_roi = m_image(make_rect<cv::Rect>(roi));

    cv::Mat hsv, bin;
    cv::cvtColor(img_roi, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, color_scale.first, color_scale.second, bin);
    int count = cv::countNonZero(bin);

    // Log.trace("oper_cooling_analyze |", count);
    return count >= cooling_task_ptr->special_params.front();
}

int BattlefieldMatcher::oper_cost_analyze(const Rect& roi) const
{
    int cost = -1;
    RegionOCRer cost_analyzer(m_image, roi);
    cost_analyzer.set_replace(Task.get<OcrTaskInfo>("NumberOcrReplace")->replace_map);
    cost_analyzer.set_use_char_model(true);
    cost_analyzer.set_bin_threshold(80, 255);
    if (!cost_analyzer.analyze()) {
        Log.warn("oper cost analyze failed");
        return cost;
    }
    if (!utils::chars_to_number(cost_analyzer.get_result().text, cost)) {
        Log.warn("oper cost convert failed, str:", cost_analyzer.get_result().text);
        return cost;
    }
    return cost;
}

bool BattlefieldMatcher::oper_available_analyze(const Rect& roi) const
{
    cv::Mat hsv;
    cv::cvtColor(m_image(make_rect<cv::Rect>(roi)), hsv, cv::COLOR_BGR2HSV);
    cv::Scalar avg = cv::mean(hsv);
    // Log.trace("oper available, mean", avg[2]);

    const int thres = Task.get("BattleOperAvailable")->special_params.front();
    if (avg[2] < thres) {
        return false;
    }
    return true;
}

bool BattlefieldMatcher::hp_flag_analyze() const
{
    // 识别 HP 的那个蓝白色图标
    Matcher flag_analyzer(m_image);
    flag_analyzer.set_task_info("BattleHpFlag");
    if (flag_analyzer.analyze()) {
        return true;
    }

    // 漏怪的时候，那个图标会变成红色的，所以多识别一次
    flag_analyzer.set_task_info("BattleHpFlag2");
    return flag_analyzer.analyze().has_value();
}

bool BattlefieldMatcher::kills_flag_analyze() const
{
    Matcher flag_analyzer(m_image);
    flag_analyzer.set_task_info("BattleKillsFlag");
    return flag_analyzer.analyze().has_value();
}

std::optional<std::pair<int, int>> BattlefieldMatcher::kills_analyze() const
{
    TemplDetOCRer kills_analyzer(m_image);
    kills_analyzer.set_task_info("BattleKillsFlag", "BattleKills");
    kills_analyzer.set_replace(Task.get<OcrTaskInfo>("NumberOcrReplace")->replace_map);

    auto kills_opt = kills_analyzer.analyze();
    if (!kills_opt) {
        return std::nullopt;
    }
    const std::string& kills_text = kills_opt->front().text;

    size_t pos = kills_text.find('/');
    if (pos == std::string::npos) {
        Log.warn("cannot found flag /");
        // 这种时候绝大多数是把 "0/41" 中的 '/' 识别成了别的什么东西（其中又有绝大部分情况是识别成了 '1'）
        // 所以这里依赖 m_pre_total_kills 转一下
        if (m_total_kills_prompt <= 0) {
            // 第一次识别就识别错了，识别成了 "0141"
            if (kills_text.at(0) != '0') {
                Log.error("m_total_kills_prompt is zero");
                return std::nullopt;
            }
            pos = 1;
        }
        else {
            size_t pre_pos = kills_text.find(std::to_string(m_total_kills_prompt));
            if (pre_pos == std::string::npos || pre_pos == 0) {
                Log.error("can't get pre_pos");
                return std::nullopt;
            }
            Log.trace("pre total kills pos:", pre_pos);
            pos = pre_pos - 1;
        }
    }

    // 例子中的"0"
    std::string kills_count = kills_text.substr(0, pos);
    if (kills_count.empty() || !ranges::all_of(kills_count, [](char c) -> bool { return std::isdigit(c); })) {
        return std::nullopt;
    }
    int kills = std::stoi(kills_count);

    // 例子中的"41"
    std::string total_kills_text = kills_text.substr(pos + 1, std::string::npos);
    int total_kills = 0;
    if (total_kills_text.empty() || !ranges::all_of(total_kills_text, [](char c) -> bool { return std::isdigit(c); })) {
        Log.warn("total kills recognition failed, set to", m_total_kills_prompt);
        total_kills = m_total_kills_prompt;
    }
    else {
        total_kills = std::stoi(total_kills_text);
    }
    total_kills = std::max(total_kills, m_total_kills_prompt);

    Log.trace("Kills:", kills, "/", total_kills);
    return std::make_pair(kills, total_kills);
}

bool BattlefieldMatcher::cost_symbol_analyze() const
{
    Matcher flag_analyzer(m_image);
    flag_analyzer.set_task_info("BattleCostFlag");
    return flag_analyzer.analyze().has_value();
}

std::optional<int> BattlefieldMatcher::costs_analyze() const
{
    RegionOCRer cost_analyzer(m_image);
    cost_analyzer.set_task_info("BattleCostData");
    cost_analyzer.set_replace(Task.get<OcrTaskInfo>("NumberOcrReplace")->replace_map);

    auto cost_opt = cost_analyzer.analyze();
    if (!cost_opt) {
        return std::nullopt;
    }
    const std::string& cost_str = cost_opt->text;

    if (cost_str.empty() || !ranges::all_of(cost_str, [](const char& c) -> bool { return std::isdigit(c); })) {
        return std::nullopt;
    }
    return std::stoi(cost_str);
}

std::optional<double> BattlefieldMatcher::cost_regeneration_analyze() const
{
    const auto& cost_regeneration_task_ptr = Task.get<MatchTaskInfo>("CostRegenerationBar");
    BrightPointAnalyzer analyzer(m_image);
    analyzer.set_roi(cost_regeneration_task_ptr->roi);
    if (!analyzer.analyze()) {
        return 0;
    }
    return static_cast<double>(analyzer.get_result().size()) / cost_regeneration_task_ptr->special_params[0];
}

bool BattlefieldMatcher::pause_button_analyze() const
{
    auto task_ptr = Task.get("BattleHasStarted");
    cv::Mat roi = m_image(make_rect<cv::Rect>(task_ptr->roi));
    cv::Mat roi_gray;
    cv::cvtColor(roi, roi_gray, cv::COLOR_BGR2GRAY);
    cv::Mat bin;
    const int value_threshold = task_ptr->special_params[0];
    cv::threshold(roi_gray, bin, value_threshold, 255, cv::THRESH_BINARY);
    int count = cv::countNonZero(bin);
    const int count_threshold = task_ptr->special_params[1];
    Log.trace(__FUNCTION__, "count", count, "threshold", count_threshold);

#ifdef ASST_DEBUG
    cv::rectangle(m_image_draw, make_rect<cv::Rect>(task_ptr->roi), cv::Scalar(0, 0, 255), 2);
    cv::putText(
        m_image_draw,
        std::to_string(count) + "/" + std::to_string(count_threshold),
        cv::Point(task_ptr->roi.x, task_ptr->roi.y + task_ptr->roi.height + 10),
        cv::FONT_HERSHEY_PLAIN,
        1.2,
        cv::Scalar(255, 255, 255),
        2);
#endif

    return count > count_threshold;
}

bool BattlefieldMatcher::in_detail_analyze() const
{
    auto analyze = [&](const std::string& task_name) {
        auto task_ptr = Task.get(task_name);
        cv::Mat roi = m_image(make_rect<cv::Rect>(task_ptr->roi));
        cv::Mat roi_hsv;
        cv::cvtColor(roi, roi_hsv, cv::COLOR_BGR2HSV);
        cv::Mat bin1;
        cv::inRange(roi_hsv, cv::Scalar(99, 235, 235), cv::Scalar(105, 255, 255), bin1);
        int count1 = cv::countNonZero(bin1);

        cv::Mat bin2;
        cv::inRange(roi_hsv, cv::Scalar(99, 235, 135), cv::Scalar(105, 255, 155), bin2);
        int count2 = cv::countNonZero(bin2);

        const int threshold = task_ptr->special_params[0];
        Log.info("in_detail, count:", count1, count2, ", threshold:", threshold);

        return count1 > threshold || count2 > threshold;
    };

    if (!analyze("BattleOperDetailPageFlag") && !analyze("BattleOperDetailPageOldFlag")) {
        return false;
    }
    return true;
}

bool asst::BattlefieldMatcher::speed_button_analyze() const
{
    auto task_ptr = Task.get("BattleSpeedButton");
    cv::Mat roi = m_image(make_rect<cv::Rect>(task_ptr->roi));
    cv::Mat roi_gray;
    cv::cvtColor(roi, roi_gray, cv::COLOR_BGR2GRAY);
    cv::Mat bin;
    const int value_threshold = task_ptr->special_params[0];
    cv::threshold(roi_gray, bin, value_threshold, 255, cv::THRESH_BINARY);
    int count = cv::countNonZero(bin);
    const int count_threshold = task_ptr->special_params[1];
    Log.trace(__FUNCTION__, "count", count, "threshold", count_threshold);

#ifdef ASST_DEBUG
    cv::rectangle(m_image_draw, make_rect<cv::Rect>(task_ptr->roi), cv::Scalar(0, 0, 255), 2);
    cv::putText(
        m_image_draw,
        std::to_string(count) + "/" + std::to_string(count_threshold),
        cv::Point(task_ptr->roi.x, task_ptr->roi.y + task_ptr->roi.height + 20),
        cv::FONT_HERSHEY_PLAIN,
        1.2,
        cv::Scalar(255, 255, 255),
        2);
#endif

    return count > count_threshold;
}
