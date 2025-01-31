#include "AdbController.h"

#include "Assistant.h"
#include "Common/AsstConf.h"
#include "Utils/NoWarningCV.h"
#include <cpr/cpr.h>
#include <cstdint>
#include <numeric>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4068)
#endif
#include <zlib/decompress.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "Common/AsstTypes.h"
#include "Config/GeneralConfig.h"
#include "Utils/Logger.hpp"
#include "Utils/Platform.hpp"
#include "Utils/StringMisc.hpp"

#include <regex>

asst::AdbController::AdbController(const AsstCallback& callback, Assistant* inst, PlatformType type) :
    InstHelper(inst),
    m_callback(callback)
{
    LogTraceFunction;

    m_platform_io = PlatformFactory::create_platform(inst, type);

    if (!m_platform_io) {
        Log.error("platform not supported");
        throw std::runtime_error("platform not supported");
    }

    m_support_socket = m_platform_io->m_support_socket;

    if (!m_support_socket) {
        Log.error("socket not supported");
    }
}

asst::AdbController::~AdbController()
{
    LogTraceFunction;

    m_inited = false;
    release();
}

std::optional<std::string> asst::AdbController::reconnect(const std::string& cmd, int64_t timeout, bool recv_by_socket)
{
    LogTraceFunction;

    json::value reconnect_info = json::object {
        { "uuid", m_uuid },
        { "what", "Reconnecting" },
        { "why", "" },
        { "details",
          json::object {
              { "reconnect", m_adb.connect },
              { "cmd", cmd },
          } },
    };
    static constexpr int ReconnectTimes = 5;
    for (int i = 0; i < ReconnectTimes; ++i) {
        if (need_exit()) {
            break;
        }
        reconnect_info["details"]["times"] = i;
        callback(AsstMsg::ConnectionInfo, reconnect_info);

        sleep(10 * 1000);
        if (need_exit()) {
            break;
        }
        auto reconnect_ret = call_command(m_adb.connect, 60LL * 1000, false /* 禁止重连避免无限递归 */);
        if (need_exit()) {
            break;
        }
        bool is_reconnect_success = false;
        if (reconnect_ret) {
            auto& reconnect_str = reconnect_ret.value();
            is_reconnect_success = reconnect_str.find("error") == std::string::npos;
        }
        if (is_reconnect_success) {
            auto recall_ret = call_command(cmd, timeout, false /* 禁止重连避免无限递归 */, recv_by_socket);
            if (recall_ret) {
                // 重连并成功执行了
                reconnect_info["what"] = "Reconnected";
                callback(AsstMsg::ConnectionInfo, reconnect_info);
                return recall_ret;
            }
        }
    }
    json::value info = json::object {
        { "uuid", m_uuid },
        { "what", "Disconnect" },
        { "why", "Reconnect failed" },
        { "details",
          json::object {
              { "cmd", m_adb.connect },
          } },
    };
    m_inited = false; // 重连失败，释放
    release();
    callback(AsstMsg::ConnectionInfo, info);

    return std::nullopt;
}

std::optional<std::string> asst::AdbController::call_command(
    const std::string& cmd,
    int64_t timeout,
    bool allow_reconnect,
    bool recv_by_socket)
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    // LogTraceScope(std::string(__FUNCTION__) + " | `" + cmd + "`");

    std::string pipe_data;
    std::string sock_data;
    asst::platform::single_page_buffer<char> pipe_buffer;
    asst::platform::single_page_buffer<char> sock_buffer;

    auto start_time = steady_clock::now();
    std::unique_lock<std::mutex> callcmd_lock(m_callcmd_mutex);

    std::optional<int> exit_res;

    exit_res = m_platform_io->call_command(cmd, recv_by_socket, pipe_data, sock_data, timeout, start_time);

    if (!exit_res) {
        Log.warn("Call `", cmd, "` failed");
        return std::nullopt;
    }
    const int exit_ret = exit_res.value();

    callcmd_lock.unlock();

    m_last_command_duration = duration_cast<milliseconds>(steady_clock::now() - start_time).count();
    Log.info(
        "Call `",
        cmd,
        "` ret",
        exit_ret,
        ", cost",
        m_last_command_duration,
        "ms , stdout size:",
        pipe_data.size(),
        ", socket size:",
        sock_data.size());
    if (!pipe_data.empty() && pipe_data.size() < 4096) {
        m_pipe_data_size = pipe_data.size();
        Log.trace("stdout output:", Logger::separator::newline, pipe_data);
    }
    if (recv_by_socket && !sock_data.empty() && sock_data.size() < 4096) {
        Log.trace("socket output:", Logger::separator::newline, sock_data);
    }
    // 直接 return，避免走到下面的 else if 里的 m_inited = false) 关闭 adb 连接，
    // 导致停止后再开始任务还需要重连一次
    if (need_exit()) {
        return std::nullopt;
    }

    if (!exit_ret) {
        return recv_by_socket ? sock_data : pipe_data;
    }
    else if (inited() && allow_reconnect) {
        // 之前可以运行，突然运行不了了，这种情况多半是 adb 炸了。所以重新连接一下
        return reconnect(cmd, timeout, recv_by_socket);
    }

    return std::nullopt;
}

size_t asst::AdbController::get_pipe_data_size() const noexcept
{
    return m_pipe_data_size;
}

size_t asst::AdbController::get_version() const noexcept
{
    return m_version;
}

void asst::AdbController::callback(AsstMsg msg, const json::value& details)
{
    if (m_callback) {
        m_callback(msg, details, m_inst);
    }
}

int asst::AdbController::get_mumu_index(const std::string& address)
{
    LogTrace << VAR(address);

    auto pos = address.find(":");
    if (pos == std::string::npos) {
        Log.error("address is invalid", address);
        return 0;
    }

    std::string port_str = address.substr(pos + 1);
    if (port_str.empty() || !ranges::all_of(port_str, [](const char& c) -> bool { return std::isdigit(c); })) {
        Log.error("port is invalid", port_str);
        return 0;
    }
    int port = std::stoi(port_str);
    int mumu_index = (port - 16384) / 32; // must be int
    LogInfo << VAR(port_str) << VAR(port) << VAR(mumu_index);
    return mumu_index;
}

void asst::AdbController::init_mumu_extras(const AdbCfg& adb_cfg, const std::string& address)
{
#if !ASST_WITH_EMULATOR_EXTRAS
    std::ignore = adb_cfg;
    std::ignore = address;
    Log.error("MaaCore is not compiled with ASST_WITH_EMULATOR_EXTRAS");
#else
    if (adb_cfg.extras.empty()) {
        LogWarn << "adb_cfg.extras is empty";
        return;
    }

    set_mumu_package(adb_cfg.extras.get("client_type", ""));

    std::filesystem::path mumu_path = utils::path(adb_cfg.extras.get("path", ""));

    if (adb_cfg.extras.contains("index")) { // MuMu index is provided directly
        m_mumu_extras.init(mumu_path, adb_cfg.extras.get("index", 0));
    }
    else {
        m_mumu_extras.init(mumu_path, get_mumu_index(address));
    }
#endif
}

void asst::AdbController::set_mumu_package(const std::string& client_type)
{
#if !ASST_WITH_EMULATOR_EXTRAS
    std::ignore = client_type;
    Log.error("MaaCore is not compiled with ASST_WITH_EMULATOR_EXTRAS");
#else
    std::string package_name = Config.get_package_name(client_type).value_or("");
    m_mumu_extras.set_package_name(package_name);
#endif
}

void asst::AdbController::init_ld_extras(const AdbCfg& adb_cfg [[maybe_unused]])
{
#if !ASST_WITH_EMULATOR_EXTRAS
    Log.error("MaaCore is not compiled with ASST_WITH_EMULATOR_EXTRAS");
#else
    if (adb_cfg.extras.empty()) {
        LogWarn << "adb_cfg.extras is empty";
        return;
    }

    std::filesystem::path ld_path = utils::path(adb_cfg.extras.get("path", ""));
    int ld_index = adb_cfg.extras.get("index", 0);
    int ld_pid = adb_cfg.extras.get("pid", 0);
    m_ld_extras.init(ld_path, ld_index, ld_pid, m_width, m_height);
#endif
}

void asst::AdbController::close_socket() noexcept
{
    m_platform_io->close_socket();
}

std::optional<unsigned short> asst::AdbController::init_socket(const std::string& local_address)
{
    return m_platform_io->init_socket(local_address);
}

void asst::AdbController::clear_info() noexcept
{
    m_inited = false;
    m_adb = decltype(m_adb)();
    m_uuid.clear();
    m_width = 0;
    m_height = 0;
    m_screen_size = { 0, 0 };
}

bool asst::AdbController::inited() const noexcept
{
    return m_inited;
}

bool asst::AdbController::start_game(const std::string& client_type)
{
    if (client_type.empty()) {
        return false;
    }
    auto package_name = Config.get_package_name(client_type);
    if (!package_name) {
        return false;
    }

    std::string cur_cmd = utils::string_replace_all(m_adb.start, "[PackageName]", package_name.value());
    bool ret = call_command(cur_cmd).has_value();

    return ret;
}

bool asst::AdbController::stop_game(const std::string& client_type)
{
    if (client_type.empty()) {
        return false;
    }
    auto package_name = Config.get_package_name(client_type);
    if (!package_name) {
        return false;
    }
    std::string cur_cmd = utils::string_replace_all(m_adb.stop, "[PackageName]", package_name.value());
    bool ret = call_command(cur_cmd).has_value();

    return ret;
}

bool asst::AdbController::click(const Point& p)
{
    if (p.x < 0 || p.x >= m_width || p.y < 0 || p.y >= m_height) {
        Log.error("click point out of range");
    }

    std::string cur_cmd =
        utils::string_replace_all(m_adb.click, { { "[x]", std::to_string(p.x) }, { "[y]", std::to_string(p.y) } });
    return call_command(cur_cmd).has_value();
}

bool asst::AdbController::input(const std::string& text)
{
    if (text == "") {
        Log.error("empty text");
    }

    std::string cur_cmd = utils::string_replace_all(m_adb.input, { { "[text]", text } });
    return call_command(cur_cmd).has_value();
}

bool asst::AdbController::swipe(
    const Point& p1,
    const Point& p2,
    int duration,
    bool extra_swipe,
    [[maybe_unused]] double slope_in,
    [[maybe_unused]] double slope_out,
    [[maybe_unused]] bool with_pause)
{
    int x1 = p1.x, y1 = p1.y;
    int x2 = p2.x, y2 = p2.y;

    // 起点不能在屏幕外，但是终点可以
    if (x1 < 0 || x1 >= m_width || y1 < 0 || y1 >= m_height) {
        Log.warn("swipe point1 is out of range", x1, y1);
        x1 = std::clamp(x1, 0, m_width - 1);
        y1 = std::clamp(y1, 0, m_height - 1);
    }

    const auto& opt = Config.get_options();

    std::string duration_str =
        duration <= 0 ? "" : std::to_string(static_cast<int>(duration * opt.adb_swipe_duration_multiplier));
    std::string cur_cmd = utils::string_replace_all(
        m_adb.swipe,
        {
            { "[x1]", std::to_string(x1) },
            { "[y1]", std::to_string(y1) },
            { "[x2]", std::to_string(x2) },
            { "[y2]", std::to_string(y2) },
            { "[duration]", duration_str },
        });
    bool ret = call_command(cur_cmd).has_value();

    // 额外的滑动：adb有bug，同样的参数，偶尔会划得非常远。额外做一个短程滑动，把之前的停下来
    if (extra_swipe && opt.adb_extra_swipe_duration > 0) {
        std::string extra_cmd = utils::string_replace_all(
            m_adb.swipe,
            {
                { "[x1]", std::to_string(x2) },
                { "[y1]", std::to_string(y2) },
                { "[x2]", std::to_string(x2) },
                { "[y2]", std::to_string(y2 - opt.adb_extra_swipe_dist /* * m_control_scale*/) },
                { "[duration]", std::to_string(opt.adb_extra_swipe_duration) },
            });
        ret &= call_command(extra_cmd).has_value();
    }
    return ret;
}

bool asst::AdbController::press_esc()
{
    LogTraceFunction;

    return call_command(m_adb.press_esc).has_value();
}

std::pair<int, int> asst::AdbController::get_screen_res() const noexcept
{
    return m_screen_size;
}

void asst::AdbController::release()
{
    close_socket();

    if (m_kill_adb_on_exit && !m_adb.release.empty()) {
        m_platform_io->release_adb(m_adb.release, 20000);
    }
}

const std::string& asst::AdbController::get_uuid() const
{
    return m_uuid;
}

bool asst::AdbController::convert_lf(std::string& data)
{
    if (data.empty() || data.size() < 2) {
        return false;
    }
    auto pred = [](const std::string::iterator& cur) -> bool {
        return *cur == '\r' && *(cur + 1) == '\n';
    };
    // find the first of "\r\n"
    auto first_iter = data.end();
    for (auto iter = data.begin(); iter != data.end() - 1; ++iter) {
        if (pred(iter)) {
            first_iter = iter;
            break;
        }
    }
    if (first_iter == data.end()) {
        return false;
    }
    // move forward all non-crlf elements
    auto end_r1_iter = data.end() - 1;
    auto next_iter = first_iter;
    while (++first_iter != end_r1_iter) {
        if (!pred(first_iter)) {
            *next_iter = *first_iter;
            ++next_iter;
        }
    }
    *next_iter = *end_r1_iter;
    ++next_iter;
    data.erase(next_iter, data.end());
    return true;
}

bool asst::AdbController::screencap(cv::Mat& image_payload, bool allow_reconnect)
{
    using namespace std::chrono;
    DecodeFunc decode_raw = [&](const std::string& data) -> bool {
        if (data.size() < 8) {
            return false;
        }
        // assuming little endian
        uint32_t w = static_cast<uint32_t>(static_cast<unsigned char>(data[0])) << 0 |
                     static_cast<uint32_t>(static_cast<unsigned char>(data[1])) << 8 |
                     static_cast<uint32_t>(static_cast<unsigned char>(data[2])) << 16 |
                     static_cast<uint32_t>(static_cast<unsigned char>(data[3])) << 24;
        uint32_t h = static_cast<uint32_t>(static_cast<unsigned char>(data[4])) << 0 |
                     static_cast<uint32_t>(static_cast<unsigned char>(data[5])) << 8 |
                     static_cast<uint32_t>(static_cast<unsigned char>(data[6])) << 16 |
                     static_cast<uint32_t>(static_cast<unsigned char>(data[7])) << 24;
        if (int(w) != m_width || int(h) != m_height) {
            Log.error("Size from image header", w, h, "does not match the size of screen", m_width, m_height);
            return false;
        }
        size_t std_size = 4ULL * m_width * m_height;
        if (data.size() < std_size) {
            return false;
        }
        const size_t header_size = data.size() - std_size; // 12 or 16. ref:
        // https://android.googlesource.com/platform/frameworks/base/+/26a2b97dbe48ee45e9ae70110714048f2f360f97%5E%21/cmds/screencap/screencap.cpp
        auto img_data_beg = data.cbegin() + header_size;
        cv::Mat temp(m_height, m_width, CV_8UC4, const_cast<char*>(&*img_data_beg));
        if (temp.empty()) {
            return false;
        }
        const auto& br = *(temp.end<cv::Vec4b>() - 1);
        if (br[3] != 255) { // only check alpha
            return false;
        }
        cv::cvtColor(temp, temp, cv::COLOR_RGBA2BGR);
        image_payload = temp;
        return true;
    };

    DecodeFunc decode_raw_with_gzip = [&](const std::string& data) -> bool {
        const std::string raw_data = gzip::decompress(data.data(), data.size());
        return decode_raw(raw_data);
    };

    DecodeFunc decode_encode = [&](const std::string& data) -> bool {
        cv::Mat temp = cv::imdecode({ data.data(), int(data.size()) }, cv::IMREAD_COLOR);
        if (temp.empty()) {
            return false;
        }
        image_payload = temp;
        return true;
    };

    image_payload = cv::Mat(); // 清空缓存
    if (m_adb.screencap_method == AdbProperty::ScreencapMethod::UnknownYet) {
        // TODO: write a DroidCast benchmark method
        Log.info("Try to find the fastest way to screencap");
        auto min_cost = milliseconds(LLONG_MAX);
        clear_lf_info();

        auto start_time = steady_clock::now();
        if (m_support_socket && m_server_started &&
            screencap(m_adb.screencap_raw_by_nc, decode_raw, allow_reconnect, true, 5000)) {
            auto duration = duration_cast<milliseconds>(steady_clock::now() - start_time);
            if (duration < min_cost) {
                m_adb.screencap_method = AdbProperty::ScreencapMethod::RawByNc;
                m_inited = true;
                min_cost = duration;
            }
            Log.info("RawByNc cost", duration.count(), "ms");
        }
        else {
            Log.info("RawByNc is not supported");
        }
        clear_lf_info();

        start_time = steady_clock::now();
        if (screencap(m_adb.screencap_raw_with_gzip, decode_raw_with_gzip, allow_reconnect)) {
            auto duration = duration_cast<milliseconds>(steady_clock::now() - start_time);
            if (duration < min_cost) {
                m_adb.screencap_method = AdbProperty::ScreencapMethod::RawWithGzip;
                m_inited = true;
                min_cost = duration;
            }
            Log.info("RawWithGzip cost", duration.count(), "ms");
        }
        else {
            Log.info("RawWithGzip is not supported");
        }
        clear_lf_info();

        start_time = steady_clock::now();
        if (screencap(m_adb.droidcast_screencap_addr, decode_encode, allow_reconnect)) {
            auto duration = duration_cast<milliseconds>(steady_clock::now() - start_time);
            if (duration < min_cost) {
                m_adb.screencap_method = AdbProperty::ScreencapMethod::RawByDroidCast;
                m_inited = true;
                min_cost = duration;
            }
            Log.info("ScreenCap_Raw cost", duration.count(), "ms");
        }
        else {
            Log.info("ScreenCap_Raw is not supported");
        }
        clear_lf_info();

        start_time = steady_clock::now();
        if (screencap(m_adb.screencap_encode, decode_encode, allow_reconnect)) {
            auto duration = duration_cast<milliseconds>(steady_clock::now() - start_time);
            if (duration < min_cost) {
                m_adb.screencap_method = AdbProperty::ScreencapMethod::Encode;
                m_inited = true;
                min_cost = duration;
            }
            Log.info("Encode cost", duration.count(), "ms");
        }
        else {
            Log.info("Encode is not supported");
        }

#if ASST_WITH_EMULATOR_EXTRAS
        if (m_mumu_extras.inited()) {
            start_time = steady_clock::now();
            if (m_mumu_extras.screencap()) {
                auto duration = duration_cast<milliseconds>(steady_clock::now() - start_time);
                if (duration < min_cost) {
                    m_adb.screencap_method = AdbProperty::ScreencapMethod::MumuExtras;
                    m_inited = true;
                    min_cost = duration;
                }
                Log.info("MumuExtras cost", duration.count(), "ms");
            }
            else {
                Log.info("MumuExtras is not supported");
            }
        }
        if (m_ld_extras.inited()) {
            start_time = steady_clock::now();
            if (m_ld_extras.screencap()) {
                auto duration = duration_cast<milliseconds>(steady_clock::now() - start_time);
                if (duration < min_cost) {
                    m_adb.screencap_method = AdbProperty::ScreencapMethod::LDExtras;
                    m_inited = true;
                    min_cost = duration;
                }
                Log.info("LDExtras cost", duration.count(), "ms");
            }
            else {
                Log.info("LDExtras is not supported");
            }
        }
#endif

        static const std::unordered_map<AdbProperty::ScreencapMethod, std::string> MethodName = {
            { AdbProperty::ScreencapMethod::UnknownYet, "UnknownYet" },
            { AdbProperty::ScreencapMethod::RawByNc, "RawByNc" },
            { AdbProperty::ScreencapMethod::RawByDroidCast, "RawByDroidCast" },
            { AdbProperty::ScreencapMethod::RawWithGzip, "RawWithGzip" },
            { AdbProperty::ScreencapMethod::Encode, "Encode" },
#if ASST_WITH_EMULATOR_EXTRAS
            { AdbProperty::ScreencapMethod::MumuExtras, "MumuExtras" },
            { AdbProperty::ScreencapMethod::LDExtras, "LDExtras" },
#endif
        };
        Log.info("The fastest way is", MethodName.at(m_adb.screencap_method), ", cost:", min_cost.count(), "ms");
        if (m_adb.screencap_method != AdbProperty::ScreencapMethod::UnknownYet) {
            json::value info = json::object {
                { "uuid", m_uuid },
                { "what", "FastestWayToScreencap" },
                { "details",
                  json::object {
                      { "method", MethodName.at(m_adb.screencap_method) },
                      { "cost", min_cost.count() },
                  } },
            };
            callback(AsstMsg::ConnectionInfo, info);
        }
        clear_lf_info();
        return m_adb.screencap_method != AdbProperty::ScreencapMethod::UnknownYet;
    }
    else {
        auto start_time = high_resolution_clock::now();
        bool screencap_ret = false;
        switch (m_adb.screencap_method) {
        case AdbProperty::ScreencapMethod::RawByNc:
            screencap_ret = screencap(m_adb.screencap_raw_by_nc, decode_raw, allow_reconnect, true);
            break;
        case AdbProperty::ScreencapMethod::RawByDroidCast:
            screencap_ret = screencap(m_adb.droidcast_screencap_addr, decode_raw, allow_reconnect, true);
            break;
        case AdbProperty::ScreencapMethod::RawWithGzip:
            screencap_ret = screencap(m_adb.screencap_raw_with_gzip, decode_raw_with_gzip, allow_reconnect);
            break;
        case AdbProperty::ScreencapMethod::Encode:
            screencap_ret = screencap(m_adb.screencap_encode, decode_encode, allow_reconnect);
            break;
#if ASST_WITH_EMULATOR_EXTRAS
        case AdbProperty::ScreencapMethod::MumuExtras: {
            auto img_opt = m_mumu_extras.screencap();
            screencap_ret = img_opt.has_value();

            if (!screencap_ret && allow_reconnect) {
                m_mumu_extras.reload();
                img_opt = m_mumu_extras.screencap();
                screencap_ret = img_opt.has_value();
            }

            if (screencap_ret) {
                image_payload = img_opt.value();
            }
        } break;
        case AdbProperty::ScreencapMethod::LDExtras: {
            auto img_opt = m_ld_extras.screencap();
            screencap_ret = img_opt.has_value();

            if (!screencap_ret && allow_reconnect) {
                m_ld_extras.reload();
                img_opt = m_ld_extras.screencap();
                screencap_ret = img_opt.has_value();
            }

            if (screencap_ret) {
                image_payload = img_opt.value();
            }
        } break;
#endif
        default:
            break;
        }
        auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start_time);
        // 记录截图耗时，每10次截图回传一次最值+平均值
        m_screencap_cost.emplace_back(screencap_ret ? duration.count() : -1); // 记录截图耗时
        ++m_screencap_times;

        if (m_screencap_cost.size() > 30) {
            m_screencap_cost.pop_front();
        }
        if (m_screencap_times > 9) { // 每 10 次截图计算一次平均耗时
            m_screencap_times = 0;
            auto filtered_cost = m_screencap_cost | views::filter([](auto num) { return num > 0; });
            if (filtered_cost.empty()) {
                return screencap_ret;
            }
            // 过滤后的有效截图用时次数
            auto filtered_count = m_screencap_cost.size() - ranges::count(m_screencap_cost, -1);
            auto [screencap_cost_min, screencap_cost_max] = ranges::minmax(filtered_cost);
            json::value info = json::object {
                { "uuid", m_uuid },
                { "what", "ScreencapCost" },
                { "details",
                  json::object {
                      { "min", screencap_cost_min },
                      { "max", screencap_cost_max },
                      { "avg",
                        filtered_count > 0
                            ? std::accumulate(filtered_cost.begin(), filtered_cost.end(), 0ll) / filtered_count
                            : -1 },
                  } },
            };
            if (m_screencap_cost.size() - filtered_count > 0) {
                info["details"]["fault_times"] = m_screencap_cost.size() - filtered_count;
            }
            callback(AsstMsg::ConnectionInfo, info);
        }
        return screencap_ret;
    }
}

bool asst::AdbController::screencap(
    const std::string& cmd,
    const DecodeFunc& decode_func,
    bool allow_reconnect,
    bool by_socket,
    int timeout)
{
    if ((!m_support_socket || !m_server_started) && by_socket) [[unlikely]] {
        return false;
    }
    std::optional<std::string> ret;
    if (cmd.rfind("http", 0) == 0) {
        // cmd以"http"开头，跳过
        ret = get_image_from_droidcast(cmd);
    }
    else {
        // 否则，执行call_command
        ret = call_command(cmd, timeout, allow_reconnect, by_socket);
    }

    if (!ret || ret.value().empty()) [[unlikely]] {
        Log.warn("data is empty!");
        return false;
    }
    auto& data = ret.value();

    bool tried_conversion = false;
    if (m_adb.screencap_end_of_line == AdbProperty::ScreencapEndOfLine::CRLF) {
        tried_conversion = true;
        if (!convert_lf(data)) [[unlikely]] { // 没找到 "\r\n"
            Log.info("screencap_end_of_line is set to CRLF but no `\\r\\n` found, set it to LF");
            m_adb.screencap_end_of_line = AdbProperty::ScreencapEndOfLine::LF;
        }
    }

    if (decode_func(data)) [[likely]] {
        if (m_adb.screencap_end_of_line == AdbProperty::ScreencapEndOfLine::UnknownYet) [[unlikely]] {
            Log.info("screencap_end_of_line is LF");
            m_adb.screencap_end_of_line = AdbProperty::ScreencapEndOfLine::LF;
        }
    }
    else {
        Log.info("data is not empty, but image is empty");

        if (tried_conversion) { // 已经转换过行尾，再次转换 data 不会变化，不必重试
            Log.error("skip retry decoding and decode failed!");
            return false;
        }

        Log.info("try to cvt lf");
        if (!convert_lf(data)) { // 没找到 "\r\n"，data 没有变化，不必重试
            Log.error("no `\\r\\n` found, skip retry decode");
            return false;
        }
        if (!decode_func(data)) {
            Log.error("convert lf and retry decode failed!");
            return false;
        }

        if (m_adb.screencap_end_of_line == AdbProperty::ScreencapEndOfLine::UnknownYet) {
            Log.info("screencap_end_of_line is CRLF");
        }
        else {
            Log.info("screencap_end_of_line is changed to CRLF");
        }
        m_adb.screencap_end_of_line = AdbProperty::ScreencapEndOfLine::CRLF;
    }
    return true;
}

bool asst::AdbController::init_droidcast(
    const AdbCfg& adb_cfg,
    std::function<std::string(const std::string&)> cmd_replace)
{
    auto release_droidcast = [&]([[maybe_unused]] bool force) {
        m_droidcast_handler.reset();
    };
    auto call_and_hup_droidcast = [&]() {
        release_droidcast(true);

        std::string cmd = m_adb.call_droidcast;
        Log.info(cmd);

        std::string pipe_str;

        m_droidcast_handler = m_platform_io->interactive_shell(cmd);
        if (!m_droidcast_handler) {
            Log.error("unable to start droidcast");
            return false;
        }

        auto check_timeout = [&](const auto& start_time) -> bool {
            using namespace std::chrono_literals;
            return std::chrono::steady_clock::now() - start_time < 3s;
        };

        const auto start_time = std::chrono::steady_clock::now();
        while (true) {
            if (need_exit()) {
                release_droidcast(true);
                return false;
            }

            pipe_str += m_droidcast_handler->read(3);

            if (!check_timeout(start_time)) {
                Log.info("unable to find > from pipe_str:", Logger::separator::newline, pipe_str);
                release_droidcast(true);
                return false;
            }

            if (pipe_str.find('>') != std::string::npos) {
                break;
            }
        }

        Log.info("pipe str", Logger::separator::newline, pipe_str);

        // NOTE: here the droidcast really start up
        // m_droidcaster =
        //     std::make_unique<DroidCaster>(std::bind(&DroidCastController::input_to_droidcast, this,
        //     std::placeholders::_1));

        return true;
    };
    auto probe_droidcast = [&](const AdbCfg& adb_cfg, std::function<std::string(const std::string&)> cmd_replace) {
        auto cast_uuid = m_uuid + "_cast.apk";

        auto droidcast_cmd_rep = [&](const std::string& cfg_cmd) -> std::string {
            using namespace asst::utils::path_literals;
            return utils::string_replace_all(
                cmd_replace(cfg_cmd),
                {
                    { "[droidCastRawLocalPath]",
                      utils::path_to_utf8_string(ResDir.get() / "droidcast_raw"_p / "droidcast_raw.apk"_p) },
                    { "[droidCastRawWorkingFile]", cast_uuid },
                });
        };

        if (!call_command(droidcast_cmd_rep(adb_cfg.push_droidcast))) {
            return false;
        }
        if (!call_command(droidcast_cmd_rep(adb_cfg.chmod_droidcast))) {
            return false;
        }
        if (!call_command(droidcast_cmd_rep(adb_cfg.reforward_droidcast_port))) {
            return false;
        }

        m_adb.call_droidcast = droidcast_cmd_rep(adb_cfg.call_droidcast);

        if (!call_and_hup_droidcast()) {
            return false;
        }

        return true;
    };
    return probe_droidcast(adb_cfg, cmd_replace);
}

std::optional<std::string> asst::AdbController::get_image_from_droidcast(const std::string& addr)
{
    using namespace std::chrono;

    Log.trace("Function get_image_from_droidcast called with addr: " + addr);

    auto start_time = steady_clock::now();
    std::unique_lock<std::mutex> callcmd_lock(m_callcmd_mutex);

    std::string response_data;
    try {
        // 发起 HTTP GET 请求
        cpr::Response r = cpr::Get(cpr::Url { addr });

        // 检查响应状态码
        if (r.error) {
            Log.warn("HTTP request failed: " + r.error.message);
            return std::nullopt;
        }

        if (r.status_code != 200) {
            Log.warn("Unexpected HTTP status code: " + std::to_string(r.status_code));
            return std::nullopt;
        }

        // 检查返回的数据是否为空
        if (r.text.empty()) {
            Log.warn("Response is empty");
            return std::nullopt;
        }

        response_data = r.text;
    }
    catch (const std::exception& e) {
        Log.error("Exception occurred in get_image_from_droidcast: " + std::string(e.what()));
        return std::nullopt;
    }
    catch (...) {
        Log.error("Unknown exception occurred in get_image_from_droidcast");
        return std::nullopt;
    }

    auto duration = duration_cast<milliseconds>(steady_clock::now() - start_time).count();
    Log.info(
        "HTTP request to `" + addr + "` succeeded, response size: " + std::to_string(response_data.size()) +
        " bytes, duration: " + std::to_string(duration) + "ms");

    if (response_data.size() < 4096) {
        Log.trace("Response content: " + response_data);
    }

    return response_data;
}

bool asst::AdbController::connect(const std::string& adb_path, const std::string& address, const std::string& config)
{
    LogTraceFunction;

    clear_info();

#ifdef ASST_DEBUG
    if (config == "DEBUG") {
        m_inited = true;
        return true;
    }
#endif

    auto get_info_json = [&]() -> json::value {
        return json::object {
            { "uuid", m_uuid },
            { "details",
              json::object {
                  { "adb", adb_path },
                  { "address", address },
                  { "config", config },
              } },
        };
    };

    auto adb_ret = Config.get_adb_cfg(config);
    if (!adb_ret) {
        json::value info = get_info_json() | json::object {
            { "what", "ConnectFailed" },
            { "why", "ConfigNotFound" },
        };
        callback(AsstMsg::ConnectionInfo, info);
#ifdef ASST_DEBUG
        return false;
#else
        Log.error("config ", config, "not found");
        adb_ret = Config.get_adb_cfg("General");
#endif
    }

    const auto& adb_cfg = adb_ret.value();
    std::string display_id;
    std::string nc_address = "10.0.2.2";
    uint16_t nc_port = 0;

    // 里面的值每次执行命令后可能更新，所以要用 lambda 拿最新的
    auto cmd_replace = [&](const std::string& cfg_cmd) -> std::string {
        return utils::string_replace_all(
            cfg_cmd,
            {
                { "[Adb]", adb_path },
                { "[AdbSerial]", address },
                { "[DisplayId]", display_id },
                { "[NcPort]", std::to_string(nc_port) },
                { "[NcAddress]", nc_address },
            });
    };

    if (need_exit()) {
        return false;
    }

    /* connect */
    {
        // 先用 devices 读取输出
        m_adb.devices = cmd_replace(adb_cfg.devices);
        m_adb.address_regex = cmd_replace(adb_cfg.address_regex);
        auto devices_ret = call_command(m_adb.devices, 60LL * 1000, false);
        bool need_connect = true;
        if (devices_ret) {
            const auto& devices_str = devices_ret.value();
            const std::regex address_regex(m_adb.address_regex);
            for (std::sregex_iterator iter(devices_str.begin(), devices_str.end(), address_regex), end; iter != end;
                 ++iter) {
                if (iter->size() > 1 && iter->str(1) == address) {
                    need_connect = false;
                    break;
                }
            }
        }

        // 如果不包含 `:` 且需要连接，connect 命令也不会成功
        if (address.find(':') == std::string::npos && need_connect) {
            json::value info = get_info_json() | json::object {
                { "what", "ConnectFailed" },
                { "why", "Address does not contain ':' and no devices found" },
            };
            callback(AsstMsg::ConnectionInfo, info);
            return false;
        }

        // TODO: adb lite server 尚未实现，第一次连接需要执行一次 adb.exe 启动 daemon
        m_adb.connect = cmd_replace(adb_cfg.connect);
        m_adb.release = cmd_replace(adb_cfg.release);
        auto connect_ret = call_command(m_adb.connect, 60LL * 1000, false /* adb 连接时不允许重试 */);
        bool is_connect_success = false;
        if (connect_ret) {
            auto& connect_str = connect_ret.value();
            // 检查连接字符串是否包含 "connected"
            is_connect_success = connect_str.find("connected") != std::string::npos;
            // NOTE:这玩意啥都没干，有什么用吗？
            if (connect_str.find("daemon started successfully") != std::string::npos &&
                connect_str.find("daemon still not running") == std::string::npos) {
            }
        }

        if (!is_connect_success && need_connect) {
            json::value info = get_info_json() | json::object {
                { "what", "ConnectFailed" },
                { "why", "Connection command failed to exec" },
            };
            callback(AsstMsg::ConnectionInfo, info);
            return false;
        }
    }

    if (need_exit()) {
        return false;
    }

    /* get uuid (imei) */
    {
        auto uuid_ret = call_command(cmd_replace(adb_cfg.uuid), 20000, false /* adb 连接时不允许重试 */);
        if (!uuid_ret) {
            json::value info = get_info_json() | json::object {
                { "what", "ConnectFailed" },
                { "why", "Uuid command failed to exec" },
            };
            callback(AsstMsg::ConnectionInfo, info);
            return false;
        }

        auto& uuid_str = uuid_ret.value();
        std::erase_if(uuid_str, [](char c) { return !std::isdigit(c) && !std::isalpha(c); });
        m_uuid = std::move(uuid_str);

        json::value info = get_info_json() | json::object {
            { "what", "UuidGot" },
            { "why", "" },
        };
        info["details"]["uuid"] = m_uuid;
        callback(AsstMsg::ConnectionInfo, info);
    }
    /* get android version */
    {
        auto version_ret = call_command(cmd_replace(adb_cfg.version));
        if (!version_ret) {
            json::value info = get_info_json() | json::object {
                { "what", "ConnectFailed" },
                { "why", "Android version command failed to exec" },
            };
            callback(AsstMsg::ConnectionInfo, info);
            return false;
        }

        auto& version_str = version_ret.value();
        convert_lf(version_str);
        m_version = std::stoul(version_str);
    }

    if (need_exit()) {
        return false;
    }

    // 按需获取display ID 信息
    if (!adb_cfg.display_id.empty()) {
        auto display_id_ret = call_command(cmd_replace(adb_cfg.display_id));
        if (!display_id_ret) {
            return false;
        }

        auto& display_id_pipe_str = display_id_ret.value();
        convert_lf(display_id_pipe_str);
        auto last = display_id_pipe_str.rfind(':');
        if (last == std::string::npos) {
            return false;
        }

        display_id = display_id_pipe_str.substr(last + 1);
        // 去掉换行
        display_id.pop_back();
    }

    if (need_exit()) {
        return false;
    }

    /* display */
    {
        auto display_ret = call_command(cmd_replace(adb_cfg.display));
        if (!display_ret) {
            json::value info = get_info_json() | json::object {
                { "what", "ConnectFailed" },
                { "why", "Display command failed to exec" },
            };
            callback(AsstMsg::ConnectionInfo, info);
            return false;
        }
        std::stringstream display_ss(display_ret.value());
        int size_value1 = 0;
        int size_value2 = 0;
        display_ss >> size_value1 >> size_value2;

        m_width = (std::max)(size_value1, size_value2);
        m_height = (std::min)(size_value1, size_value2);

        json::value info = get_info_json() | json::object {
            { "what", "ResolutionGot" },
            { "why", "" },
        };

        info["details"] |= json::object {
            { "width", m_width },
            { "height", m_height },
        };

        callback(AsstMsg::ConnectionInfo, info);

        if (m_width == 0 || m_height == 0) {
            info["what"] = "ResolutionError";
            info["why"] = "Get resolution failed";
            callback(AsstMsg::ConnectionInfo, info);
            return false;
        }
        m_screen_size = { m_width, m_height };
    }

    /* DroidCast */
    bool droidcast_available = init_droidcast(adb_cfg, cmd_replace);
    if (!droidcast_available) {
        json::value info = get_info_json() | json::object {
            { "what", "DroidCastNotAvailable" },
            { "why", "" },
        };
        callback(AsstMsg::ConnectionInfo, info);
        return false;
    }

    if (need_exit()) {
        return false;
    }

    {
        json::value info = get_info_json() | json::object {
            { "what", "Connected" },
            { "why", "" },
        };
        callback(AsstMsg::ConnectionInfo, info);
    }

    m_adb.click = cmd_replace(adb_cfg.click);
    m_adb.input = cmd_replace(adb_cfg.input);
    m_adb.swipe = cmd_replace(adb_cfg.swipe);
    m_adb.press_esc = cmd_replace(adb_cfg.press_esc);
    m_adb.screencap_raw_with_gzip = cmd_replace(adb_cfg.screencap_raw_with_gzip);
    m_adb.droidcast_screencap_addr = adb_cfg.dcr_screencap_addr;
    m_adb.screencap_encode = cmd_replace(adb_cfg.screencap_encode);
    m_adb.start = cmd_replace(adb_cfg.start);
    m_adb.stop = cmd_replace(adb_cfg.stop);
    m_adb.back_to_home = cmd_replace(adb_cfg.back_to_home);

    if (m_support_socket && !m_server_started) {
        std::string bind_address;
        if (size_t pos = address.rfind(':'); pos != std::string::npos) {
            bind_address = address.substr(0, pos);
        }
        else {
            bind_address = "127.0.0.1";
        }

        // reference from
        // https://github.com/ArknightsAutoHelper/ArknightsAutoHelper/blob/master/automator/connector/ADBConnector.py#L436
        auto nc_address_ret = call_command(cmd_replace(adb_cfg.nc_address));
        if (nc_address_ret && !m_server_started) {
            auto& nc_result_str = nc_address_ret.value();
            if (auto pos = nc_result_str.find(' '); pos != std::string::npos) {
                nc_address = nc_result_str.substr(0, pos);
            }
        }

        auto socket_opt = init_socket(bind_address);
        if (socket_opt) {
            nc_port = socket_opt.value();
            m_adb.screencap_raw_by_nc = cmd_replace(adb_cfg.screencap_raw_by_nc);
            m_server_started = true;
        }
        else {
            m_server_started = false;
        }
    }

    if (need_exit()) {
        return false;
    }

    if (config == "MuMuEmulator12") {
        init_mumu_extras(adb_cfg, address);
    }
    else if (config == "LDPlayer") {
        init_ld_extras(adb_cfg);
    }

    if (need_exit()) {
        return false;
    }

    return true;
}

void asst::AdbController::set_kill_adb_on_exit(bool enable) noexcept
{
    m_kill_adb_on_exit = enable;
}

void asst::AdbController::clear_lf_info()
{
    m_adb.screencap_end_of_line = AdbProperty::ScreencapEndOfLine::UnknownYet;
}

void asst::AdbController::back_to_home() noexcept
{
    call_command(m_adb.back_to_home);
    return;
}
