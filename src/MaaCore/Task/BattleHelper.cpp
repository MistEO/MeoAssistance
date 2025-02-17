#include "BattleHelper.h"

#include <future>
#include <thread>

#include "Config/GeneralConfig.h"
#include "Config/Miscellaneous/AvatarCacheManager.h"
#include "Config/Miscellaneous/BattleDataConfig.h"
#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/ImageIo.hpp"
#include "Utils/Logger.hpp"
#include "Utils/NoWarningCV.h"
#include "Utils/Time.hpp"
#include "Vision/Battle/BattlefieldClassifier.h"
#include "Vision/Battle/BattlefieldMatcher.h"
#include "Vision/BestMatcher.h"
#include "Vision/Matcher.h"
#include "Vision/RegionOCRer.h"

#include "Arknights-Tile-Pos/TileCalc2.hpp"

using namespace asst::battle;

asst::BattleHelper::BattleHelper(Assistant* inst) :
    m_inst_helper(inst)
{
}

bool asst::BattleHelper::set_stage_name(const std::string& name)
{
    LogTraceFunction;

    if (auto result = Tile.find(name); !result || !json::open(result->second)) {
        return false;
    }
    m_stage_name = name;
    return true;
}

void asst::BattleHelper::clear()
{
    m_side_tile_info.clear();
    m_normal_tile_info.clear();
    m_skill_usage.clear();
    m_skill_times.clear();
    m_skill_error_count.clear();
    m_last_use_skill_time.clear();
    m_camera_count = 0;
    m_camera_shift = { 0., 0. };

    m_in_battle = false;
    m_kills = 0;
    m_total_kills = 0;
    m_cur_deployment_opers.clear();
    m_battlefield_opers.clear();
    m_used_tiles.clear();
}

bool asst::BattleHelper::calc_tiles_info(const std::string& stage_name, double shift_x, double shift_y)
{
    LogTraceFunction;

    if (auto result = Tile.find(stage_name); !result || !json::open(result->second)) {
        return false;
    }

    m_map_data = TilePack::find_level(stage_name).value_or(Map::Level {});
    auto calc_result = TilePack::calc(stage_name, shift_x, shift_y);
    m_normal_tile_info = std::move(calc_result.normal_tile_info);
    m_side_tile_info = std::move(calc_result.side_tile_info);
    m_retreat_button_pos = calc_result.retreat_button;
    m_skill_button_pos = calc_result.skill_button;

    return true;
}

bool asst::BattleHelper::pause()
{
    LogTraceFunction;

    return ProcessTask(this_task(), { "BattlePause" }).run();
}

bool asst::BattleHelper::speed_up()
{
    LogTraceFunction;

    return ProcessTask(this_task(), { "BattleSpeedUp" }).run();
}

bool asst::BattleHelper::abandon()
{
    return ProcessTask(this_task(), { "RoguelikeBattleExitBegin" }).run();
}

bool asst::BattleHelper::update_deployment(bool init, const cv::Mat& reusable, bool need_oper_cost)
{
    LogTraceFunction;

    if (init) {
        AvatarCache.remove_avatars(Role::Drone); // 移除小龙等不同技能很像的召唤物，防止错误识别
        wait_until_start(false);
    }

    cv::Mat image = init || reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;

    if (init) {
        auto draw_future = std::async(std::launch::async, [&]() { save_map(image); });
    }

    BattlefieldMatcher oper_analyzer(image);

    // 保全要识别开局费用，先用init判断了，之后别的地方要用的话再做cache
    if (init || need_oper_cost) {
        oper_analyzer.set_object_of_interest({ .deployment = true, .oper_cost = true });
    }
    else {
        oper_analyzer.set_object_of_interest({ .deployment = true });
    }
    auto oper_result_opt = oper_analyzer.analyze();
    if (!oper_result_opt) {
        check_in_battle(image);
        return false;
    }
    const auto old_deployment_opers = std::move(m_cur_deployment_opers);
    m_cur_deployment_opers = std::vector<battle::DeploymentOper>();
    m_cur_deployment_opers.reserve(oper_result_opt->deployment.size());

    // 从场上干员和已占用格子中移除冷却中的干员
    auto remove_cooling_from_battlefield = [&](const DeploymentOper& oper) {
        if (!oper.cooling) {
            return;
        }
        auto iter = m_battlefield_opers.find(oper.name);
        if (iter == m_battlefield_opers.end()) {
            return;
        }
        auto loc = iter->second;
        m_used_tiles.erase(loc);
        m_battlefield_opers.erase(iter);
    };

    auto set_oper_name = [&](DeploymentOper& oper, const std::string& name) {
        oper.name = name;
        oper.location_type = BattleData.get_location_type(name);
        oper.is_usual_location = battle::get_role_usual_location(oper.role) == oper.location_type;
    };

    auto& cur_opers = oper_result_opt->deployment;
    std::vector<DeploymentOper> unknown_opers;

    for (auto& oper : cur_opers) {
        BestMatcher avatar_analyzer(oper.avatar);
        avatar_analyzer.set_method(MatchMethod::Ccoeff);
        if (oper.cooling) {
            Log.trace("start matching cooling", oper.index);
            static const auto cooling_threshold =
                Task.get<MatchTaskInfo>("BattleAvatarCoolingData")->templ_thresholds.front();
            static const auto cooling_mask_range = Task.get<MatchTaskInfo>("BattleAvatarCoolingData")->mask_ranges;
            avatar_analyzer.set_threshold(cooling_threshold);
            avatar_analyzer.set_mask_ranges(cooling_mask_range, true, true);
        }
        else {
            static const auto threshold = Task.get<MatchTaskInfo>("BattleAvatarData")->templ_thresholds.front();
            static const auto drone_threshold =
                Task.get<MatchTaskInfo>("BattleDroneAvatarData")->templ_thresholds.front();
            avatar_analyzer.set_threshold(oper.role == Role::Drone ? drone_threshold : threshold);
        }

        bool is_analyzed = false;
        if (!init) {
            for (const auto& old_oper :
                 old_deployment_opers | views::filter([&](const battle::DeploymentOper& temp_oper) {
                     return temp_oper.role == oper.role;
                 })) {
                avatar_analyzer.append_templ(old_oper.name, old_oper.avatar);
            }
            if (avatar_analyzer.analyze()) {
                set_oper_name(oper, avatar_analyzer.get_result().templ_info.name);
                remove_cooling_from_battlefield(oper);
                is_analyzed = true;
            }
        }
        if (!is_analyzed) {
            // 之前的干员都没匹配上，那就把所有的干员都加进去
            // 可能的优化: 移除之前添加的模板
            auto& avatar_cache = AvatarCache.get_avatars(oper.role);
            for (const auto& [name, avatar] : avatar_cache) {
                avatar_analyzer.append_templ(name, avatar);
            }
            if (avatar_analyzer.analyze()) {
                set_oper_name(oper, avatar_analyzer.get_result().templ_info.name);
                remove_cooling_from_battlefield(oper);
            }
            else {
                Log.info("unknown oper", oper.index);
                unknown_opers.emplace_back(oper);
            }
        }

        m_cur_deployment_opers.emplace_back(oper);

        if (oper.cooling) {
            Log.trace("stop matching cooling", oper.index);
        }
    }

    if (ranges::count_if(unknown_opers, [](const DeploymentOper& it) { return !it.cooling; }) > 0 || init) {
        // 一个都没匹配上的，挨个点开来看一下
        LogTraceScope("rec unknown opers");

        // 暂停游戏准备识别干员
        do {
            pause();
            // 在刚进入游戏的时候（画面刚刚完全亮起来的时候），点暂停是没反应的
            // 所以这里一直点，直到真的点上了为止
            if (!init || !check_pause_button()) {
                break;
            }
            std::this_thread::yield();
        } while (!m_inst_helper.need_exit());

        if (!check_in_battle(image)) {
            return false;
        }

        cv::Mat name_image;
        for (auto& oper : unknown_opers) {
            LogTraceScope("rec unknown oper: " + std::to_string(oper.index));
            if (oper.cooling) {
                Log.info("cooling oper, skip");
                oper.name = "UnknownCooling_" + std::to_string(oper.index);
                continue;
            }

            Rect oper_rect = oper.rect;
            // 点完部署区的一个干员之后，他的头像会放大；其他干员的位置都被挤开了，不在原来的位置了
            // 所以只有第一个干员可以直接点，后面干员都要重新识别一下位置
            if (!name_image.empty()) {
                Matcher re_matcher;
                re_matcher.set_task_info("BattleAvatarReMatch");
                re_matcher.set_image(name_image);
                re_matcher.set_templ(oper.avatar);
                auto rematched = re_matcher.analyze();
                if (rematched) {
                    oper_rect = rematched->rect;
                }
            }

            click_oper_on_deployment(oper_rect);

            name_image = m_inst_helper.ctrler()->get_image();
            if (!check_in_battle(name_image)) {
                return false;
            }

            std::string name = analyze_detail_page_oper_name(name_image);
            // 这时候即使名字不合法也只能凑合用了，但是为空还是不行的
            if (name.empty()) {
                Log.error("name is empty");
                continue;
            }
            set_oper_name(oper, name);
            remove_cooling_from_battlefield(oper);

            m_cur_deployment_opers[oper.index] = oper;
            AvatarCache.set_avatar(name, oper.role, oper.avatar);
        }
        pause();
        if (!unknown_opers.empty()) {
            cancel_oper_selection();
        }

        image = m_inst_helper.ctrler()->get_image();
    }

    if (init) {
        update_kills(image);
    }

    return check_in_battle(image);
}

// if side = true, get top view of the selected operator, tile size is 5x5
cv::Mat asst::BattleHelper::get_top_view(const cv::Mat& cam_img, bool side)
{
    if (!side) {
        return cv::Mat {}; // TODO
    }

    const std::vector<cv::Point2f> world_points {
        { -2.5, -2.5 },
        { 2.5, -2.5 },
        { -2.5, 2.5 },
        { 2.5, 2.5 },
    };
    const std::vector<cv::Point2f> dst_points {
        { 0, 0 },
        { 500, 0 },
        { 0, 500 },
        { 500, 500 },
    };
    std::vector<cv::Point2f> screen_points;
    for (const auto& point : world_points) {
        cv::Vec3d temp { point.x, -point.y, -0.3967874050140381 };
        auto screen_pt = Map::TileCalc2::world_to_screen(m_map_data, temp, true);
        screen_points.push_back(screen_pt);
    }

    const auto xform = cv::getPerspectiveTransform(screen_points, dst_points);
    cv::Mat result;
    cv::warpPerspective(cam_img, result, xform, cv::Size(500, 500));
    return result;
}

bool asst::BattleHelper::update_kills(const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    BattlefieldMatcher analyzer(image);
    analyzer.set_object_of_interest({ .kills = true });
    if (m_total_kills) {
        analyzer.set_total_kills_prompt(m_total_kills);
    }
    auto result_opt = analyzer.analyze();
    if (!result_opt || !result_opt->kills) {
        return false;
    }
    std::tie(m_kills, m_total_kills) = result_opt->kills.value();
    return true;
}

bool asst::BattleHelper::update_cost(const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    BattlefieldMatcher analyzer(image);
    analyzer.set_object_of_interest({ .costs = true });
    auto result_opt = analyzer.analyze();
    if (!result_opt || !result_opt->costs) {
        return false;
    }
    m_cost = result_opt->costs.value();
    return true;
}

bool asst::BattleHelper::update_cost_regenerated(const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    BattlefieldMatcher analyzer(image);
    analyzer.set_object_of_interest({ .cost_regeneration = true });
    auto result_opt = analyzer.analyze();
    if (!result_opt || !result_opt->cost_regeneration) {
        return false;
    }
    const double cost_regenerated = std::floor(m_cost_regenerated) + result_opt->cost_regeneration.value();
    if (cost_regenerated < m_cost_regenerated - 0.05) {
        m_cost_regenerated = cost_regenerated + 1;
    }
    else if (cost_regenerated > m_cost_regenerated + 0.05) {
        m_cost_regenerated = cost_regenerated;
    }
    return true;
}

bool asst::BattleHelper::deploy_oper(const std::string& name, const Point& loc, DeployDirection direction)
{
    LogTraceFunction;

    const auto swipe_oper_task_ptr = Task.get("BattleSwipeOper");
    const auto use_oper_task_ptr = Task.get("BattleUseOper");

    auto rect_opt = get_oper_rect_on_deployment(name);
    if (!rect_opt) {
        return false;
    }
    const Rect& oper_rect = *rect_opt;

    auto target_iter = m_side_tile_info.find(loc);
    if (target_iter == m_side_tile_info.cend()) {
        Log.error("No loc", loc);
        return false;
    }
    Point target_point = target_iter->second.pos;

    int dist = static_cast<int>(
        Point::distance(target_point, { oper_rect.x + oper_rect.width / 2, oper_rect.y + oper_rect.height / 2 }));

    // 1000 是随便取的一个系数，把整数的 pre_delay 转成小数用的
    int duration = static_cast<int>(dist / 1000.0 * swipe_oper_task_ptr->pre_delay);
    // 时间太短了的压根放不上去，故意加长一点
    if (int min_duration = swipe_oper_task_ptr->special_params.at(4); duration < min_duration) {
        duration = min_duration;
    }
    bool deploy_with_pause =
        ControlFeat::support(m_inst_helper.ctrler()->support_features(), ControlFeat::SWIPE_WITH_PAUSE);
    Point oper_point(oper_rect.x + oper_rect.width / 2, oper_rect.y + oper_rect.height / 2);
    m_inst_helper.ctrler()->swipe(
        oper_point,
        target_point,
        duration,
        false,
        swipe_oper_task_ptr->special_params.at(2),
        swipe_oper_task_ptr->special_params.at(3),
        deploy_with_pause);

    // 拖动干员朝向
    if (direction != DeployDirection::None) {
        static const std::unordered_map<DeployDirection, Point> DirectionMap = {
            { DeployDirection::Right, Point::right() }, { DeployDirection::Down, Point::down() },
            { DeployDirection::Left, Point::left() },   { DeployDirection::Up, Point::up() },
            { DeployDirection::None, Point::zero() },
        };

        // 计算往哪边拖动
        const Point& direction_target = DirectionMap.at(direction);

        // 将方向转换为实际的 swipe end 坐标点，并对滑动距离进行缩放
        const auto scale_size = m_inst_helper.ctrler()->get_scale_size();
        static const int coeff =
            static_cast<int>(swipe_oper_task_ptr->special_params.at(0) * scale_size.second / 720.0);
        Point end_point = target_point + (direction_target * coeff);

        fix_swipe_out_of_limit(
            target_point,
            end_point,
            scale_size.first,
            scale_size.second,
            swipe_oper_task_ptr->special_params.at(1));

        m_inst_helper.sleep(use_oper_task_ptr->post_delay);
        m_inst_helper.ctrler()->swipe(target_point, end_point, swipe_oper_task_ptr->post_delay);
        // 仅简单复用，该延迟含义与此处逻辑无关 by MistEO
        m_inst_helper.sleep(use_oper_task_ptr->pre_delay);
    }

    if (deploy_with_pause) {
        // m_inst_helper.ctrler()->press_esc();
        ProcessTask(this_task(), { "BattlePauseCancel" }).run();
    }

    // for SSS, multiple operator may be deployed at the same location.
    if (m_used_tiles.contains(loc)) {
        std::string pre_name = m_used_tiles.at(loc);
        Log.info("remove previous oper", pre_name, loc);
        m_used_tiles.erase(loc);
        m_battlefield_opers.erase(pre_name);
    }

    // 肉鸽中，一个干员可能被部署多次
    if (m_battlefield_opers.contains(name) && BattleData.get_role(name) != battle::Role::Drone) {
        Point pre_loc = m_battlefield_opers.at(name);
        Log.info("remove deploy failed oper", name, pre_loc);
        m_battlefield_opers.erase(name);
        // 擦除已使用干员,但是不擦除已使用格子
        // m_used_tiles.erase(pre_loc);
    }

    m_used_tiles.emplace(loc, name);
    m_battlefield_opers.emplace(name, loc);
    m_last_use_skill_time.emplace(loc, std::chrono::steady_clock::time_point());
    m_inst_helper.sleep(200); // 部署完会有 166 ms 的动画

    return true;
}

bool asst::BattleHelper::retreat_oper(const std::string& name)
{
    LogTraceFunction;

    auto oper_iter = m_battlefield_opers.find(name);
    if (oper_iter == m_battlefield_opers.cend()) {
        Log.error("No oper", name);
        return false;
    }

    if (!retreat_oper(oper_iter->second, false)) {
        return false;
    }

    m_battlefield_opers.erase(name);
    return true;
}

bool asst::BattleHelper::retreat_oper(const Point& loc, bool manually)
{
    LogTraceFunction;

    if (!click_oper_on_battlefield(loc) || !click_retreat()) {
        return false;
    }

    m_used_tiles.erase(loc);
    if (manually) {
        std::erase_if(m_battlefield_opers, [&loc](const auto& pair) -> bool { return pair.second == loc; });
    }
    return true;
}

bool asst::BattleHelper::is_skill_ready(const Point& loc, const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    BattlefieldClassifier skill_analyzer(image);
    skill_analyzer.set_object_of_interest({ .skill_ready = true });

    auto target_iter = m_normal_tile_info.find(loc);
    if (target_iter == m_normal_tile_info.end()) {
        Log.error("No loc", loc);
        return false;
    }
    const Point& battlefield_point = target_iter->second.pos;
    skill_analyzer.set_base_point(battlefield_point);

    return skill_analyzer.analyze()->skill_ready.ready;
}

bool asst::BattleHelper::is_skill_ready(const std::string& name, const cv::Mat& reusable)
{
    auto oper_iter = m_battlefield_opers.find(name);
    if (oper_iter == m_battlefield_opers.cend()) {
        Log.error("No oper", name);
        return false;
    }
    return is_skill_ready(oper_iter->second, reusable);
}

bool asst::BattleHelper::use_skill(const std::string& name, bool keep_waiting)
{
    LogTraceFunction;

    auto oper_iter = m_battlefield_opers.find(name);
    if (oper_iter == m_battlefield_opers.cend()) {
        Log.error("No oper", name);
        return false;
    }

    return use_skill(oper_iter->second, keep_waiting);
}

bool asst::BattleHelper::use_skill(const Point& loc, bool keep_waiting)
{
    LogTraceFunction;

    return click_oper_on_battlefield(loc) && click_skill(keep_waiting) && m_inst_helper.sleep(200);
}

bool asst::BattleHelper::check_pause_button(const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    Matcher battle_flag_analyzer(image);
    battle_flag_analyzer.set_task_info("BattleOfficiallyBegin");
    bool ret = battle_flag_analyzer.analyze().has_value();

    BattlefieldMatcher battle_flag_analyzer_2(image);
    auto battle_result_opt = battle_flag_analyzer_2.analyze();
    ret &= battle_result_opt && battle_result_opt->pause_button;
    return ret;
}

bool asst::BattleHelper::check_skip_plot_button(const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;

    Matcher battle_plot_analyzer(image);
    battle_plot_analyzer.set_task_info("SkipThePreBattlePlot");
    bool ret = battle_plot_analyzer.analyze().has_value();
    if (ret) {
        ProcessTask(this_task(), { "SkipThePreBattlePlot" }).run();
    }
    return ret;
}

bool asst::BattleHelper::check_in_speed_up(const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    Matcher analyzer(image);
    analyzer.set_task_info("BattleSpeedUpCheck");
    return analyzer.analyze().has_value();
}

bool asst::BattleHelper::check_in_battle(const cv::Mat& reusable, bool weak)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    if (weak) {
        BattlefieldMatcher analyzer(image);
        m_in_battle = analyzer.analyze().has_value();
    }
    else {
        check_skip_plot_button(image);
        m_in_battle = check_pause_button(image);
    }
    return m_in_battle;
}

bool asst::BattleHelper::wait_until_start(bool weak)
{
    LogTraceFunction;

    cv::Mat image = m_inst_helper.ctrler()->get_image();
    while (!m_inst_helper.need_exit() && !check_in_battle(image, weak)) {
        do_strategic_action(image);
        std::this_thread::yield();

        image = m_inst_helper.ctrler()->get_image();
    }
    return true;
}

bool asst::BattleHelper::wait_until_end(bool weak)
{
    LogTraceFunction;

    cv::Mat image = m_inst_helper.ctrler()->get_image();
    while (!m_inst_helper.need_exit() && check_in_battle(image, weak)) {
        do_strategic_action(image);
        std::this_thread::yield();

        image = m_inst_helper.ctrler()->get_image();
    }
    return true;
}

bool asst::BattleHelper::do_strategic_action(const cv::Mat& reusable)
{
    cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    return use_all_ready_skill(image);
}

bool asst::BattleHelper::use_all_ready_skill(const cv::Mat& reusable)
{
    // TODO: 可配置延迟时间
    static constexpr auto min_frame_interval = std::chrono::milliseconds(1500);

    bool used = false;
    const auto now = std::chrono::steady_clock::now();
    const cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    for (const auto& [name, loc] : m_battlefield_opers) {
        auto& usage = m_skill_usage[name];
        auto& retry = m_skill_error_count[name];
        auto& times = m_skill_times[name];
        auto& last_use_time = m_last_use_skill_time[name];
        if (usage != SkillUsage::Possibly && usage != SkillUsage::Times) {
            continue;
        }

        if (!is_skill_ready(loc, image)) {
            continue;
        }

        Log.info("Skill", name, "is ready");

        if (auto interval = now - last_use_time; min_frame_interval > interval) {
            Log.info(
                name,
                "use skill too fast, interval time:",
                std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(interval).count()) + " ms");
            continue;
        }

        // 识别到了，但点进去发现没有。一般来说是识别错了
        if (!use_skill(loc, false)) {
            Log.warn("Skill", name, "is not ready");
            constexpr int MaxRetry = 3;
            if (++retry >= MaxRetry) {
                Log.warn("Do not use skill anymore", name);
                usage = SkillUsage::NotUse;
            }
            continue;
        }
        used = true;
        retry = 0;
        m_last_use_skill_time[name] = now;

        if (usage == SkillUsage::Times) {
            times--;
            if (times == 0) {
                usage = SkillUsage::TimesUsed;
            }
        }
        // image = m_inst_helper.ctrler()->get_image();
    }

    return used;
}

bool asst::BattleHelper::check_and_use_skill(const std::string& name, bool& has_error, const cv::Mat& reusable)
{
    auto oper_iter = m_battlefield_opers.find(name);
    if (oper_iter == m_battlefield_opers.cend()) {
        Log.error("No oper", name);
        return false;
    }
    return check_and_use_skill(oper_iter->second, has_error, reusable);
}

bool asst::BattleHelper::check_and_use_skill(const Point& loc, bool& has_error, const cv::Mat& reusable)
{
    const cv::Mat image = reusable.empty() ? m_inst_helper.ctrler()->get_image() : reusable;
    if (!is_skill_ready(loc, image)) {
        return false;
    }
    has_error = !use_skill(loc, false);
    return true;
}

void asst::BattleHelper::save_map(const cv::Mat& image)
{
    LogTraceFunction;

    using namespace asst::utils::path_literals;
    const auto& MapRelativeDir = "debug"_p / "map"_p;

    auto draw = image.clone();
    for (const auto& [loc, info] : m_normal_tile_info) {
        cv::circle(draw, cv::Point(info.pos.x, info.pos.y), 5, cv::Scalar(0, 255, 0), -1);
        cv::putText(draw, loc.to_string(), cv::Point(info.pos.x - 30, info.pos.y), 1, 1.2, cv::Scalar(0, 0, 255), 2);
    }

    std::string suffix;
    if (++m_camera_count > 1) {
        suffix = "-" + std::to_string(m_camera_count);
    }
    asst::imwrite(MapRelativeDir / asst::utils::path(m_stage_name + suffix + ".png"), draw);
}

bool asst::BattleHelper::click_oper_on_deployment(const std::string& name)
{
    LogTraceFunction;

    auto rect_opt = get_oper_rect_on_deployment(name);
    if (!rect_opt) {
        return false;
    }

    return click_oper_on_deployment(*rect_opt);
}

bool asst::BattleHelper::click_oper_on_deployment(const Rect& rect)
{
    LogTraceFunction;

    const auto use_oper_task_ptr = Task.get("BattleUseOper");
    m_inst_helper.ctrler()->click(rect);
    m_inst_helper.sleep(use_oper_task_ptr->pre_delay);

    return true;
}

bool asst::BattleHelper::click_oper_on_battlefield(const std::string& name)
{
    LogTraceFunction;

    auto oper_iter = m_battlefield_opers.find(name);
    if (oper_iter == m_battlefield_opers.cend()) {
        Log.error("No oper", name);
        return false;
    }
    return click_oper_on_battlefield(oper_iter->second);
}

bool asst::BattleHelper::click_oper_on_battlefield(const Point& loc)
{
    LogTraceFunction;

    const auto use_oper_task_ptr = Task.get("BattleUseOper");

    auto target_iter = m_normal_tile_info.find(loc);
    if (target_iter == m_normal_tile_info.end()) {
        Log.error("No loc", loc);
        return false;
    }
    const Point& target_point = target_iter->second.pos;

    m_inst_helper.ctrler()->click(target_point);
    m_inst_helper.sleep(use_oper_task_ptr->pre_delay);

    return true;
}

bool asst::BattleHelper::click_retreat()
{
    LogTraceFunction;
    bool deploy_with_pause =
        ControlFeat::support(m_inst_helper.ctrler()->support_features(), ControlFeat::SWIPE_WITH_PAUSE);

    if (deploy_with_pause) {
        ProcessTask(this_task(), { "BattlePause" }).run();
    }
    // return ProcessTask(this_task(), { "BattleOperRetreatJustClick" }).run();
    bool ret = m_inst_helper.ctrler()->click(m_retreat_button_pos);

    if (deploy_with_pause) {
        ProcessTask(this_task(), { "BattlePauseCancel" }).run();
    }
    return ret;
}

bool asst::BattleHelper::click_skill(bool keep_waiting)
{
    LogTraceFunction;
    bool deploy_with_pause =
        ControlFeat::support(m_inst_helper.ctrler()->support_features(), ControlFeat::SWIPE_WITH_PAUSE);

    bool pausing = false;
    if (!keep_waiting && deploy_with_pause) {
        pausing = ProcessTask(this_task(), { "BattlePause" }).run();
    }

    cv::Mat top_view;
    cv::Mat image;
    for (int retry = 0; retry < (keep_waiting ? 1000 : 5); ++retry) {
        if (m_inst_helper.need_exit()) {
            return false;
        }
        image = m_inst_helper.ctrler()->get_image();
        if (keep_waiting && retry > 0 && (retry % 10 == 0) && !check_in_battle(image)) {
            return false;
        }
        top_view = get_top_view(image, true);
        Matcher skill_analyzer { top_view };
        skill_analyzer.set_task_info("BattleSkillReadyOnClick-TopView");
        skill_analyzer.set_roi({ 250, 250, 250, 250 });
        if (skill_analyzer.analyze()) {
            m_inst_helper.ctrler()->click(m_skill_button_pos);
            if (pausing) {
                ProcessTask(this_task(), { "BattlePauseCancel" }).run();
            }
            return true;
        }
        m_inst_helper.sleep(Config.get_options().task_delay);
    }

    // this means false positive in skill ready detection

#ifdef ASST_DEBUG
    if (!top_view.empty()) {
        using namespace asst::utils::path_literals;
        asst::imwrite(
            "debug"_p / "skill"_p / asst::utils::path(m_stage_name + '_' + utils::get_time_filestem() + ".png"),
            top_view);
    }
#endif
    cancel_oper_selection();
    if (pausing) {
        ProcessTask(this_task(), { "BattlePauseCancel" }).run();
    }
    return false;
}

bool asst::BattleHelper::cancel_oper_selection()
{
    return ProcessTask(this_task(), { "BattleCancelSelection" }).run();
}

void asst::BattleHelper::fix_swipe_out_of_limit(
    Point& p1,
    Point& p2,
    int width,
    int height,
    int max_distance,
    double radian)
{
    Point direct = Point::zero();
    int distance = 0;
    if (p2.y > height) {
        // 下边界超限
        direct = Point::up();
        distance = p2.y - height;
    }
    else if (p2.x > width) {
        // 右边界超限
        direct = Point::left();
        distance = p2.x - width;
    }
    else if (p2.y < 0) {
        // 上边界超限
        direct = Point::down();
        distance = -p2.y;
    }
    else if (p2.x < 0) {
        // 左边界超限
        direct = Point::right();
        distance = -p2.x;
    }
    else {
        return;
    }

    std::tuple<double, double> adjust_scale = {
        direct.x * std::cos(radian) - direct.y * std::sin(radian),
        direct.y * std::cos(radian) + direct.x * std::sin(radian),
    };

    // 旋转后偏移值会不够，计算补偿比例
    double adjust_more = std::get<0>(adjust_scale) * direct.x + std::get<1>(adjust_scale) * direct.y;

    Point adjust = {
        static_cast<int>(std::get<0>(adjust_scale) / adjust_more * distance),
        static_cast<int>(std::get<1>(adjust_scale) / adjust_more * distance),
    };

    if (auto point_distance = Point::distance(adjust, { 0, 0 }); point_distance > max_distance) {
        adjust = {
            static_cast<int>(adjust.x * max_distance / point_distance),
            static_cast<int>(adjust.y * max_distance / point_distance),
        };
    }

    Log.info(__FUNCTION__, "swipe end_point out of limit, start:", p1, ", end:", p2, ", adjust:", adjust);
    p1 += adjust;
    p2 += adjust;
}

bool asst::BattleHelper::move_camera(const std::pair<double, double>& delta)
{
    LogTraceFunction;
    Log.info("move", delta.first, delta.second);

    update_kills();

    // 还没转场的时候
    if (m_kills != 0) {
        wait_until_end(false);
    }

    m_kills = 0;
    m_total_kills = 0;

    m_camera_shift.first += delta.first;
    m_camera_shift.second += delta.second;

    calc_tiles_info(m_stage_name, -m_camera_shift.first, m_camera_shift.second);
    return update_deployment(true);
}

std::string asst::BattleHelper::analyze_detail_page_oper_name(const cv::Mat& image)
{
    const auto& replace_task = Task.get<OcrTaskInfo>("CharsNameOcrReplace");
    const auto& task = Task.get<OcrTaskInfo>(oper_name_ocr_task_name());

    RegionOCRer preproc_analyzer(image);
    preproc_analyzer.set_task_info(task);
    preproc_analyzer.set_replace(replace_task->replace_map, replace_task->replace_full);
    auto preproc_result_opt = preproc_analyzer.analyze();

    if (preproc_result_opt && !BattleData.is_name_invalid(preproc_result_opt->text)) {
        return preproc_result_opt->text;
    }

    Log.warn("ocr with preprocess got a invalid name, try to use detect model");
    OCRer det_analyzer(image);
    det_analyzer.set_task_info(task);
    det_analyzer.set_replace(replace_task->replace_map, replace_task->replace_full);
    auto det_result_opt = det_analyzer.analyze();
    if (!det_result_opt) {
        return {};
    }
    sort_by_score_(*det_result_opt);
    const auto& det_name = det_result_opt->front().text;

    return BattleData.is_name_invalid(det_name) ? std::string() : det_name;
}

std::optional<asst::Rect> asst::BattleHelper::get_oper_rect_on_deployment(const std::string& name) const
{
    LogTraceFunction;

    auto oper_iter = ranges::find_if(m_cur_deployment_opers, [&](const auto& oper) { return oper.name == name; });
    if (oper_iter == m_cur_deployment_opers.end()) {
        Log.error("No oper", name);
        return std::nullopt;
    }

    return oper_iter->rect;
}
