#pragma once
#include "Task/AbstractTask.h"
#include "Task/BattleHelper.h"

#include "Common/AsstBattleDef.h"
#include "Common/AsstTypes.h"
#include "Config/Miscellaneous/TilePack.h"

namespace asst
{
class BattleProcessTask : public AbstractTask, public BattleHelper
{
public:
    BattleProcessTask(const AsstCallback& callback, Assistant* inst, std::string_view task_chain);
    virtual ~BattleProcessTask() override = default;

    virtual bool set_stage_name(const std::string& stage_name) override;
    void set_wait_until_end(bool wait_until_end);
    void set_formation_task_ptr(std::shared_ptr<std::unordered_map<std::string, std::string>> value);

protected:
    virtual bool _run() override;

    virtual AbstractTask& this_task() override { return *this; }

    virtual void clear() override;

    virtual bool
        do_derived_action([[maybe_unused]] const battle::copilot::Action& action, [[maybe_unused]] size_t index)
    {
        return false;
    }

    virtual battle::copilot::CombatData& get_combat_data() { return m_combat_data; }

    virtual bool need_to_wait_until_end() const { return m_need_to_wait_until_end; }

    bool to_group();
    bool do_action(const battle::copilot::Action& action, size_t index);

    const std::string& get_name_from_group(const std::string& action_name);
    void notify_action(const battle::copilot::Action& action);
    bool wait_condition(const battle::copilot::Action& action);
    bool enter_bullet_time(const std::string& name, const Point& location);
    void sleep_and_do_strategy(unsigned millisecond);

    virtual bool check_in_battle(const cv::Mat& reusable = cv::Mat(), bool weak = true) override;

    battle::copilot::CombatData m_combat_data;
    std::unordered_map</*group*/ std::string, /*oper*/ std::string> m_oper_in_group;

    bool m_in_bullet_time = false;
    bool m_need_to_wait_until_end = false;
    std::shared_ptr<std::unordered_map<std::string, std::string>> m_formation_ptr = nullptr;
    std::chrono::steady_clock::time_point m_battle_starting_time;
};
}
