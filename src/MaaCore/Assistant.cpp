#include "Assistant.h"

#include "Utils/NoWarningCV.h"
#include "Utils/Ranges.hpp"
#include <meojson/json.hpp>

#include "Config/GeneralConfig.h"
#include "Config/Miscellaneous/OcrPack.h"
#include "Config/OnnxSessions.h"
#include "Config/ResourceLoader.h"
#include "Controller/Controller.h"
#include "Status.h"
#include "Task/Interface/AwardTask.h"
#include "Task/Interface/CloseDownTask.h"
#include "Task/Interface/CopilotTask.h"
#include "Task/Interface/CustomTask.h"
#include "Task/Interface/DepotTask.h"
#include "Task/Interface/FightTask.h"
#include "Task/Interface/InfrastTask.h"
#include "Task/Interface/MallTask.h"
#include "Task/Interface/OperBoxTask.h"
#include "Task/Interface/ReclamationTask.h"
#include "Task/Interface/RecruitTask.h"
#include "Task/Interface/RoguelikeTask.h"
#include "Task/Interface/SSSCopilotTask.h"
#include "Task/Interface/SingleStepTask.h"
#include "Task/Interface/StartUpTask.h"
#include "Task/Interface/VideoRecognitionTask.h"
#include "Utils/Logger.hpp"
#ifdef ASST_DEBUG
#include "Task/Interface/DebugTask.h"
#endif

using namespace asst;

bool ::AsstExtAPI::set_static_option(StaticOptionKey key, const std::string& value)
{
    Log.info(__FUNCTION__, "| key", static_cast<int>(key), "value", value);

    switch (key) {
    case StaticOptionKey::CpuOCR: {
        WordOcr::get_instance().use_cpu();
        CharOcr::get_instance().use_cpu();
        OnnxSessions::get_instance().use_cpu();
        return true;
    } break;
    case StaticOptionKey::GpuOCR: {
        int device_id = std::stoi(value);
        WordOcr::get_instance().use_gpu(device_id);
        CharOcr::get_instance().use_gpu(device_id);
        OnnxSessions::get_instance().use_gpu(device_id);
        return true;
    } break;
    default:
        Log.error(__FUNCTION__, "| unknown key:", static_cast<int>(key));
        break;
    }

    return false;
}

Assistant::Assistant(ApiCallback callback, void* callback_arg) :
    m_callback(callback),
    m_callback_arg(callback_arg)
{
    LogTraceFunction;

    m_status = std::make_shared<Status>();
    m_ctrler = std::make_shared<Controller>(append_callback_for_inst, this);

    m_msg_thread = std::thread(&Assistant::msg_proc, this);
    m_call_thread = std::thread(&Assistant::call_proc, this);
    m_working_thread = std::thread(&Assistant::working_proc, this);
}

Assistant::~Assistant()
{
    LogTraceFunction;

    // dirty stuff preventing Logger from being destructed before ResourceLoader::load_thread_func exits,
    // which creates empty files with random name on Linux. I have no idea how this could work
    ResourceLoader::get_instance().cancel();

    m_thread_exit = true;
    m_thread_idle = true;

    {
        std::unique_lock<std::mutex> lock(m_call_mutex);
        m_call_condvar.notify_all();
    }
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condvar.notify_all();
    }
    {
        std::unique_lock<std::mutex> lock(m_msg_mutex);
        m_msg_condvar.notify_all();
    }

    if (m_working_thread.joinable()) {
        m_working_thread.join();
    }
    if (m_call_thread.joinable()) {
        m_call_thread.join();
    }
    if (m_msg_thread.joinable()) {
        m_msg_thread.join();
    }
    if (m_callback) {
        m_callback(static_cast<AsstMsgId>(AsstMsg::Destroyed), "{}", m_callback_arg);
    }
}

bool asst::Assistant::set_instance_option(InstanceOptionKey key, const std::string& value)
{
    Log.info(__FUNCTION__, "| key", static_cast<int>(key), "value", value);
    switch (key) {
    case InstanceOptionKey::TouchMode:
        if (constexpr std::string_view Adb = "adb"; value == Adb) {
            m_ctrler->set_touch_mode(TouchMode::Adb);
            return true;
        }
        else if (constexpr std::string_view Minitouch = "minitouch"; value == Minitouch) {
            m_ctrler->set_touch_mode(TouchMode::Minitouch);
            return true;
        }
        else if (constexpr std::string_view MaaTouch = "maatouch"; value == MaaTouch) {
            m_ctrler->set_touch_mode(TouchMode::Maatouch);
            return true;
        }
        else if (constexpr std::string_view MacPlayTools = "MacPlayTools"; value == MacPlayTools) {
            m_ctrler->set_touch_mode(TouchMode::MacPlayTools);
            return true;
        }
        break;
    case InstanceOptionKey::DeploymentWithPause:
        if (constexpr std::string_view Enable = "1"; value == Enable) {
            m_ctrler->set_swipe_with_pause(true);
            return true;
        }
        else if (constexpr std::string_view Disable = "0"; value == Disable) {
            m_ctrler->set_swipe_with_pause(false);
            return true;
        }
        break;
    case InstanceOptionKey::AdbLiteEnabled:
        if (constexpr std::string_view Enable = "1"; value == Enable) {
            m_ctrler->set_adb_lite_enabled(true);
            return true;
        }
        else if (constexpr std::string_view Disable = "0"; value == Disable) {
            m_ctrler->set_adb_lite_enabled(false);
            return true;
        }
        break;
    case InstanceOptionKey::KillAdbOnExit:
        if (constexpr std::string_view Enable = "1"; value == Enable) {
            m_ctrler->set_kill_adb_on_exit(true);
            return true;
        }
        else if (constexpr std::string_view Disable = "0"; value == Disable) {
            m_ctrler->set_kill_adb_on_exit(false);
            return true;
        }
        break;
    default:
        break;
    }
    Log.error("Unknown key or value", value);
    return false;
}

bool asst::Assistant::ctrl_connect(const std::string& adb_path, const std::string& address, const std::string& config)
{
    LogTraceFunction;

    std::unique_lock<std::mutex> lock(m_mutex);

    // 仍有任务进行，connect 前需要 stop
    if (!m_thread_idle) {
        return false;
    }

    m_thread_idle = false;

    bool ret = m_ctrler->connect(adb_path, address, config.empty() ? "General" : config);
    if (ret) {
        m_uuid = m_ctrler->get_uuid();
    }

    m_thread_idle = true;
    return ret;
}

bool asst::Assistant::ctrl_click(int x, int y)
{
    return m_ctrler->click(Point(x, y));
}

bool asst::Assistant::ctrl_screencap()
{
    return m_ctrler->screencap();
}

asst::Assistant::TaskId asst::Assistant::append_task(const std::string& type, const std::string& params)
{
    Log.info(__FUNCTION__, type, params);

    auto ret = json::parse(params.empty() ? "{}" : params);
    if (!ret) {
        Log.error("json::parse failed");
        return 0;
    }

    std::shared_ptr<InterfaceTask> ptr = nullptr;

#define ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(TASK)        \
    else if (type == TASK::TaskType)                                  \
    {                                                                 \
        ptr = std::make_shared<TASK>(append_callback_for_inst, this); \
    }

    if constexpr (false) {
    }
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(FightTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(StartUpTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(CloseDownTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(AwardTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(MallTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(InfrastTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(RecruitTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(RoguelikeTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(CopilotTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(SSSCopilotTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(SingleStepTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(VideoRecognitionTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(DepotTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(OperBoxTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(ReclamationTask)
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(CustomTask)
#ifdef ASST_DEBUG
    ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH(DebugTask)
#endif
    else
    {
        Log.error(__FUNCTION__, "| invalid type:", type);
        return 0;
    }

#undef ASST_ASSISTANT_APPEND_TASK_FROM_STRING_IF_BRANCH

    auto& json = ret.value();
    ptr->set_enable(json.get("enable", true));

    bool params_ret = ptr->set_params(json);
    if (!params_ret) {
        Log.error(__FUNCTION__, "| invalid params:", params);
        return 0;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    int task_id = ++m_task_id;
    ptr->set_task_id(task_id);
    m_tasks_list.emplace_back(task_id, ptr);
    Log.info(__FUNCTION__, "| task_id:", task_id);

    return task_id;
}

bool asst::Assistant::set_task_params(TaskId task_id, const std::string& params)
{
    Log.info(__FUNCTION__, task_id, params);

    if (task_id <= 0) {
        return false;
    }

    auto ret = json::parse(params.empty() ? json::value().to_string() : params);
    if (!ret) {
        return false;
    }
    auto& json = ret.value();

    bool is_set = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto&& [id, ptr] : m_tasks_list) {
        if (id != task_id) {
            continue;
        }
        bool enable = json.get("enable", true);
        ptr->set_enable(enable);
        is_set = ptr->set_params(json);
        break;
    }

    return is_set;
}

std::vector<uchar> asst::Assistant::get_image() const
{
    if (!inited()) {
        return {};
    }
    cv::Mat img = m_ctrler->get_image_cache();
    std::vector<uchar> buf;
    cv::imencode(".png", img, buf);
    return buf;
}

bool asst::Assistant::connect(const std::string& adb_path, const std::string& address, const std::string& config)
{
    LogTraceFunction;

    return ctrl_connect(adb_path, address, config);
}

asst::Assistant::AsyncCallId asst::Assistant::async_connect(
    const std::string& adb_path,
    const std::string& address,
    const std::string& config,
    bool block)
{
    LogTraceFunction;

    return append_async_call(
        AsyncCallItem::Type::Connect,
        AsyncCallItem::ConnectParams { .adb_path = adb_path, .address = address, .config = config },
        block);
}

asst::Assistant::AsyncCallId asst::Assistant::async_click(int x, int y, bool block)
{
    LogTraceFunction;

    return append_async_call(AsyncCallItem::Type::Click, AsyncCallItem::ClickParams { .x = x, .y = y }, block);
}

asst::Assistant::AsyncCallId asst::Assistant::async_screencap(bool block)
{
    LogTraceFunction;

    return append_async_call(AsyncCallItem::Type::Screencap, AsyncCallItem::ScreencapParams {}, block);
}

bool asst::Assistant::connected() const
{
    return inited();
}

std::string asst::Assistant::get_uuid() const
{
    return m_uuid;
}

std::vector<Assistant::TaskId> asst::Assistant::get_tasks_list() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<TaskId> result(m_tasks_list.size());
    ranges::copy(m_tasks_list | views::keys, result.begin());
    return result;
}

bool asst::Assistant::start(bool block)
{
    LogTraceFunction;
    Log.info("Start |", block ? "block" : "non block");

    if (!m_thread_idle) {
        return false;
    }
    std::unique_lock<std::mutex> lock;
    if (block) { // 外部调用
        lock = std::unique_lock<std::mutex>(m_mutex);
    }
    m_thread_idle = false;
    m_running = true;
    m_condvar.notify_one();

    return true;
}

bool Assistant::stop(bool block)
{
    LogTraceFunction;
    Log.info("Stop |", block ? "block" : "non block");

    m_thread_idle = true;

    std::unique_lock<std::mutex> lock;
    if (block) { // 外部调用
        lock = std::unique_lock<std::mutex>(m_mutex);
    }
    m_tasks_list.clear();

    clear_cache();
    return true;
}

bool asst::Assistant::running() const
{
    return m_running;
}

void Assistant::working_proc()
{
    LogTraceFunction;

    std::vector<TaskId> finished_tasks;
    while (true) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_thread_exit) {
            m_running = false;
            return;
        }

        if (m_thread_idle || m_tasks_list.empty()) {
            finished_tasks.clear();
            m_thread_idle = true;
            m_running = false;
            Log.rotate_check();
            m_condvar.wait(lock);
            continue;
        }

        m_running = true;
        const auto [id, task_ptr] = m_tasks_list.front();
        lock.unlock();
        // only one instance of working_proc running, unlock here to allow set_task_param to the running task

        json::value callback_json = json::object {
            { "taskchain", std::string(task_ptr->get_task_chain()) },
            { "taskid", id },
        };
        append_callback(AsstMsg::TaskChainStart, callback_json);

        bool ret = task_ptr->run();
        finished_tasks.emplace_back(id);

        lock.lock();
        if (!m_tasks_list.empty()) {
            m_tasks_list.pop_front();
        }
        lock.unlock();

        auto msg =
            m_thread_idle ? AsstMsg::TaskChainStopped : (ret ? AsstMsg::TaskChainCompleted : AsstMsg::TaskChainError);
        append_callback(msg, callback_json);

        if (m_thread_idle) {
            finished_tasks.clear();
            continue;
        }

        if (m_tasks_list.empty()) {
            callback_json["finished_tasks"] = json::array(finished_tasks);
            append_callback(AsstMsg::AllTasksCompleted, callback_json);
            finished_tasks.clear();
            clear_cache();
        }

        const int delay = Config.get_options().task_delay;
        lock.lock();
        m_condvar.wait_for(lock, std::chrono::milliseconds(delay), [&]() -> bool { return m_thread_idle; });

        if (m_thread_idle) {
            append_callback(AsstMsg::TaskChainStopped, callback_json);
        }
    }
}

void Assistant::msg_proc()
{
    LogTraceFunction;

    while (true) {
        std::unique_lock<std::mutex> lock(m_msg_mutex);
        if (m_thread_exit) {
            return;
        }

        if (m_msg_queue.empty()) {
            m_msg_condvar.wait(lock);
            continue;
        }

        auto [msg, detail] = std::move(m_msg_queue.front());
        m_msg_queue.pop();
        lock.unlock();

        if (m_callback) {
            m_callback(static_cast<AsstMsgId>(msg), detail.to_string().c_str(), m_callback_arg);
        }
    }
}

asst::Assistant::AsyncCallId
    asst::Assistant::append_async_call(AsyncCallItem::Type type, AsyncCallItem::Parmas params, bool block)
{
    LogTraceFunction;

    AsyncCallId id = 0;
    {
        std::unique_lock<std::mutex> lock(m_call_mutex);
        id = ++m_call_id;
        AsyncCallItem item { .id = id, .type = type, .params = std::move(params) };

        m_call_queue.emplace(std::move(item));
        m_call_condvar.notify_one();
    }

    if (block) {
        // 需要保证队列中id一定是有序的
        wait_async_id(id);
    }

    return id;
}

bool asst::Assistant::wait_async_id(AsyncCallId id)
{
    while (true) {
        std::unique_lock<std::mutex> lock(m_completed_call_mutex);
        if (m_thread_exit) {
            return false;
        }

        // 需要保证队列中id一定是有序的
        if (id <= m_completed_call) {
            return true;
        }
        m_completed_call_condvar.wait(lock);
    }
    return false;
}

void asst::Assistant::call_proc()
{
    LogTraceFunction;

    while (true) {
        std::unique_lock<std::mutex> lock(m_call_mutex);
        if (m_thread_exit) {
            return;
        }

        if (m_call_queue.empty()) {
            m_call_condvar.wait(lock);
            continue;
        }

        auto call_item = std::move(m_call_queue.front());
        m_call_queue.pop();
        lock.unlock();

        auto start = std::chrono::steady_clock::now();
        bool ret = false;
        std::string what;

        switch (call_item.type) {
        case AsyncCallItem::Type::Connect: {
            what = "Connect";
            const auto& [adb_path, address, config] = std::get<AsyncCallItem::ConnectParams>(call_item.params);
            ret = ctrl_connect(adb_path, address, config);
        } break;
        case AsyncCallItem::Type::Click: {
            what = "Click";
            const auto& [x, y] = std::get<AsyncCallItem::ClickParams>(call_item.params);
            ret = ctrl_click(x, y);
        } break;
        case AsyncCallItem::Type::Screencap: {
            what = "Screencap";
            std::ignore = std::get<AsyncCallItem::ScreencapParams>(call_item.params);
            ret = ctrl_screencap();
        } break;
        default:
            what = "Unknown";
            ret = false;
            break;
        }

        {
            std::unique_lock<std::mutex> completed_call_lock(m_completed_call_mutex);
            m_completed_call = call_item.id;
            m_completed_call_condvar.notify_all();
        }

        auto cost =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        json::value cb_info = json::object {
            { "uuid", m_uuid },
            { "what", what },
            { "async_call_id", call_item.id },
            {
                "details",
                json::object {
                    { "ret", ret },
                    { "cost", cost },
                },
            },
        };
        append_callback(AsstMsg::AsyncCallInfo, cb_info);
    }
}

void Assistant::append_callback(AsstMsg msg, const json::value& detail)
{
    json::value more_detail = detail;
    if (!more_detail.contains("uuid")) {
        more_detail["uuid"] = m_uuid;
    }
    switch (msg) {
    case AsstMsg::InternalError:
    case AsstMsg::InitFailed:
        stop(false);
        break;
    default:
        break;
    }

    // 加入回调消息队列，由回调消息线程外抛给外部
    Log.info("Assistant::append_callback |", msg, more_detail.to_string());

    std::unique_lock<std::mutex> lock(m_msg_mutex);
    m_msg_queue.emplace(msg, std::move(more_detail));
    m_msg_condvar.notify_one();
}

void asst::Assistant::append_callback_for_inst(AsstMsg msg, const json::value& detail, Assistant* inst)
{
    if (!inst) {
        return;
    }
    inst->append_callback(msg, detail);
}

void Assistant::clear_cache()
{
    m_status->clear_number();
    m_status->clear_rect();
    m_status->clear_str();
}

bool asst::Assistant::inited() const noexcept
{
    return m_ctrler && m_ctrler->inited();
}

bool asst::Assistant::back_to_home() const
{
    return m_ctrler->back_to_home();
}
