#include "CopilotTask.h"

#include <regex>

#include "Config/Miscellaneous/CopilotConfig.h"
#include "Config/TaskData.h"
#include "Task/Fight/MedicineCounterTaskPlugin.h"
#include "Task/Miscellaneous/BattleFormationTask.h"
#include "Task/Miscellaneous/BattleProcessTask.h"
#include "Task/Miscellaneous/TaskFileReloadTask.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Utils/Platform.hpp"

asst::CopilotTask::CopilotTask(const AsstCallback& callback, Assistant* inst) :
    InterfaceTask(callback, inst, TaskType),
    m_task_file_reload_task_ptr(std::make_shared<TaskFileReloadTask>(callback, inst, TaskType)),
    m_navigate_task_ptr(std::make_shared<ProcessTask>(callback, inst, TaskType)),
    m_not_use_prts_task_ptr(std::make_shared<ProcessTask>(callback, inst, TaskType)),
    m_change_difficulty_task_ptr(std::make_shared<ProcessTask>(callback, inst, TaskType)),
    m_formation_task_ptr(std::make_shared<BattleFormationTask>(callback, inst, TaskType)),
    m_battle_task_ptr(std::make_shared<BattleProcessTask>(callback, inst, TaskType)),
    m_stop_task_ptr(std::make_shared<ProcessTask>(callback, inst, TaskType))
{
    LogTraceFunction;

    m_subtasks.emplace_back(m_task_file_reload_task_ptr);
    m_subtasks.emplace_back(m_navigate_task_ptr);

    m_not_use_prts_task_ptr->set_tasks({ "NotUsePrts" }).set_ignore_error(true).set_retry_times(0);
    m_subtasks.emplace_back(m_not_use_prts_task_ptr);

    // 选择突袭模式
    m_change_difficulty_task_ptr->set_tasks({ "RaidConfirm", "ChangeToRaidDifficulty" });
    m_subtasks.emplace_back(m_change_difficulty_task_ptr);

    auto start_1_tp = std::make_shared<ProcessTask>(callback, inst, TaskType);
    start_1_tp->set_tasks({ "BattleStartPre" }).set_retry_times(0).set_ignore_error(true);
    m_subtasks.emplace_back(start_1_tp);

    m_medicine_task_ptr = std::make_shared<ProcessTask>(callback, inst, TaskType);
    m_medicine_task_ptr->set_tasks({ "BattleStartPre@UseMedicine" }).set_retry_times(0).set_ignore_error(true);
    m_medicine_task_ptr->register_plugin<MedicineCounterTaskPlugin>()->set_count(999999);
    m_subtasks.emplace_back(m_medicine_task_ptr);

    m_subtasks.emplace_back(m_formation_task_ptr)->set_retry_times(0);

    auto start_2_tp = std::make_shared<ProcessTask>(callback, inst, TaskType);
    start_2_tp->set_tasks({ "BattleStartAll" }).set_ignore_error(false);
    m_subtasks.emplace_back(start_2_tp);

    // 跳过“以下干员出战后将被禁用，是否继续？”对话框
    auto start_3_tp = std::make_shared<ProcessTask>(callback, inst, TaskType);
    start_3_tp->set_tasks({ "SkipForbiddenOperConfirm", "Stop" }).set_ignore_error(false);
    m_subtasks.emplace_back(start_3_tp);

    m_subtasks.emplace_back(m_battle_task_ptr)->set_retry_times(0);

    m_stop_task_ptr->set_enable(false);
    m_subtasks.emplace_back(m_stop_task_ptr);
}

bool asst::CopilotTask::set_params(const json::value& params)
{
    LogTraceFunction;

    using SupportUnitUsage = BattleFormationTask::SupportUnitUsage;

    if (m_running) {
        return false;
    }

    bool need_navigate = params.get("need_navigate", false); // 是否在当前页面左右滑动寻找关卡，启用战斗列表则为true
    std::string navigate_name = params.get("navigate_name", std::string()); // 导航的关卡名
    bool is_raid = params.get("is_raid", false);                            // 是否为突袭关卡
    if (params.contains("is_adverse")) {
        Log.warn("================  DEPRECATED  ================");
        Log.warn("`is_adverse` has been deprecated since v5.0.0-beta.3; Please use 'is_raid'");
        Log.warn("================  DEPRECATED  ================");
        if (!params.contains("is_raid")) {
            // 兼容旧版本，在v6.0改为存在此参数时直接return false
            is_raid = params.get("is_adverse", false);
        }
    }
    bool use_sanity_potion = params.get("use_sanity_potion", false); // 是否吃理智药
    bool with_formation = params.get("formation", false);            // 是否使用自动编队
    unsigned squad_index = params.get("select_formation", 0);        // 选择第几个编队，0为不选择
    bool add_trust = params.get("add_trust", false);                 // 是否自动补信赖
    bool add_user_additional = params.contains("user_additional");   // 是否自动补用户自定义干员
    auto support_unit_usage = static_cast<SupportUnitUsage>(
        params.get("support_unit_usage", static_cast<int>(SupportUnitUsage::None))); // 助战干员使用模式
    std::string support_unit_name = params.get("support_unit_name", std::string());

    if (params.contains("add_user_additional")) {
        Log.warn("================  DEPRECATED  ================");
        Log.warn("`add_user_additional` has been deprecated since v5.1.0-beta.1;");
        Log.warn("================  DEPRECATED  ================");
    }

    auto filename_opt = params.find<std::string>("filename");
    if (!filename_opt) {
        Log.error("CopilotTask set_params failed, stage_name or filename not found");
        return false;
    }

    static const std::regex maa_regex(R"(^maa://(\d+)$)");
    std::smatch match;

    m_task_file_reload_task_ptr->set_enable(need_navigate);
    if (std::regex_match(*filename_opt, match, maa_regex)) {
        if (!Copilot.parse_magic_code(match[1].str())) {
            Log.error("CopilotConfig parse failed");
            return false;
        }
        m_task_file_reload_task_ptr->set_magic_code(match[1].str());
    }
    else {
        if (!Copilot.load(utils::path(*filename_opt))) {
            Log.error("CopilotConfig parse failed");
            return false;
        }
        m_task_file_reload_task_ptr->set_path(std::move(*filename_opt));
    }

    m_stage_name = Copilot.get_stage_name();
    if (!m_battle_task_ptr->set_stage_name(m_stage_name)) {
        Log.error("Not support stage");
        return false;
    }

    if (!navigate_name.empty()) {
        Task.get<OcrTaskInfo>(navigate_name + "@Copilot@ClickStageName")->text = { navigate_name };
        std::string replace_navigate_name = navigate_name;
        utils::string_replace_all_in_place(replace_navigate_name, { { "-", "" } });
        Task.get<OcrTaskInfo>(navigate_name + "@Copilot@ClickedCorrectStage")->text = { navigate_name,
                                                                                        replace_navigate_name };
        m_navigate_task_ptr->set_tasks({ navigate_name + "@Copilot@StageNavigationBegin" });
    }
    else {
        m_navigate_task_ptr->set_tasks({});
    }

    m_navigate_task_ptr->set_enable(need_navigate);
    m_change_difficulty_task_ptr->set_enable(need_navigate && is_raid);
    m_not_use_prts_task_ptr->set_enable(need_navigate); // 不使用代理指挥
    m_medicine_task_ptr->set_enable(use_sanity_potion);

    // 关卡名含有"TR"的为教学关,不需要编队
    m_formation_task_ptr->set_enable(with_formation && navigate_name.find("TR") == std::string::npos);
    m_formation_task_ptr->set_squad_index(squad_index);
    if (add_trust) {
        m_formation_task_ptr->append_supplementary_oper_req(
            BattleFormationTask::SupplementaryOperReq { .sort_key = BattleFormationTask::SortKey::Trust,
                                                        .ascending = true,
                                                        .num = UINT_MAX });
    }
    m_formation_task_ptr->set_support_unit_usage(support_unit_usage);
    if (support_unit_usage == SupportUnitUsage::Specific &&
        !m_formation_task_ptr->set_specific_support_unit(support_unit_name)) {
        return false;
    }

    if (auto opt = params.find<json::array>("user_additional"); add_user_additional && opt) {
        std::vector<BattleFormationTask::RequiredOper> additional_req;
        for (const auto& op : *opt) {
            std::string name = op.get("name", std::string());
            if (name.empty()) {
                continue;
            }
            additional_req.emplace_back(
                BattleFormationTask::RequiredOper { .name = std::move(name), .skill = op.get("skill", 0) });
        }
        m_formation_task_ptr->set_additional_reqs(additional_req);
    }

    m_battle_task_ptr->set_wait_until_end(need_navigate);
    m_battle_task_ptr->set_formation_task_ptr(m_formation_task_ptr->get_formation());

    size_t loop_times = params.get("loop_times", 1);
    if (need_navigate) {
        // 如果没三星就中止
        Task.get<OcrTaskInfo>("Copilot@BattleStartPreFlag")->text.emplace_back(navigate_name);
        m_stop_task_ptr->set_tasks({ "Copilot@ClickCornerUntilEndOfAction" });
        m_stop_task_ptr->set_enable(true);
    }
    else if (loop_times > 1) {
        m_stop_task_ptr->set_tasks({ "ClickCornerUntilStartButton" });
        m_stop_task_ptr->set_enable(true);

        // 追加
        m_subtasks.reserve(m_subtasks.size() * loop_times);
        auto raw_end = m_subtasks.end();
        for (size_t i = 1; i < loop_times; ++i) {
            // FIXME: 如果多次调用 set_params，这里复制的会有问题
            m_subtasks.insert(m_subtasks.end(), m_subtasks.begin(), raw_end);
        }
    }

    return true;
}
