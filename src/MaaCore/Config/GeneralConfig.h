#pragma once

#include "AbstractConfig.h"

#include <optional>
#include <string>
#include <unordered_map>

namespace asst
{
struct RequestInfo
{
    std::string url; // 上传地址
    std::string drop_url;
    std::string recruit_url;
    std::unordered_map<std::string, std::string> headers; // 请求头
    int timeout = 0;                                      // 超时时间
};

struct DepotExportTemplate
{
    std::string ark_planner;
};

struct DebugConf
{
    int clean_files_freq = 50;
    int max_debug_file_num = 100;
};

struct Options
{
    int task_delay = 0;                         // 任务间延时：越快操作越快，但会增加CPU消耗
    int sss_fight_screencap_interval = 0;       // 保全战斗截图最小间隔
    int roguelike_fight_screencap_interval = 0; // 肉鸽战斗截图最小间隔
    int copilot_fight_screencap_interval = 0;   // 抄作业战斗截图最小间隔
    int control_delay_lower = 0;                // 点击随机延时下限：每次点击操作会进行随机延时
    int control_delay_upper = 0;                // 点击随机延时上限：每次点击操作会进行随机延时
    // bool print_window = false;// 截图功能：开启后每次结算界面会截图到screenshot目录下
    int adb_extra_swipe_dist = 0; // 额外的滑动距离：
    // adb有bug，同样的参数，偶尔会划得非常远。
    // 额外做一个短程滑动，把之前的停下来。
    int adb_extra_swipe_duration = -1; // 额外的滑动持续时间：
    // adb有bug，同样的参数，偶尔会划得非常远。
    // 额外做一个短程滑动，把之前的停下来。
    // 若小于0，则关闭额外滑动功能。
    double adb_swipe_duration_multiplier = 0;   // adb 滑动持续时间倍数
    double adb_swipe_x_distance_multiplier = 0; // adb 滑动距离倍数
    int minitouch_extra_swipe_dist = 0;
    int minitouch_extra_swipe_duration = -1;
    int minitouch_swipe_default_duration = 0;
    int minitouch_swipe_extra_end_delay = 0;
    int swipe_with_pause_required_distance = 0;
    std::vector<std::string> minitouch_programs_order;
    RequestInfo penguin_report; // 企鹅物流汇报：每次到结算界面，汇报掉落数据至企鹅物流 https://penguin-stats.io
    DepotExportTemplate depot_export_template; // 仓库识别结果导出模板
    RequestInfo yituliu_report; // 一图流大数据汇报：目前只有公招功能，https://ark.yituliu.cn/survey/maarecruitdata
    DebugConf debug;
};

struct AdbCfg
{
    /* command */
    std::string devices;
    std::string address_regex;
    std::string connect;
    std::string display_id;
    std::string uuid;
    std::string click;
    std::string input;
    std::string swipe;
    std::string press_esc;
    std::string display;
    std::string screencap_raw_with_gzip;
    std::string screencap_raw_by_nc;
    std::string dcr_screencap_addr;
    std::string nc_address;
    std::string screencap_encode;
    std::string release;
    std::string start;
    std::string stop;
    std::string abilist;
    std::string version;
    std::string orientation;
    std::string push_minitouch;
    std::string chmod_minitouch;
    std::string call_minitouch;
    std::string call_maatouch;
    std::string push_droidcast;
    std::string call_droidcast;
    std::string chmod_droidcast;
    std::string reforward_droidcast_port;
    std::string back_to_home;
    json::object extras;
};

class GeneralConfig final : public SingletonHolder<GeneralConfig>, public AbstractConfig
{
public:
    virtual ~GeneralConfig() override = default;

    void set_connection_extras(const std::string& name, const json::object& diff);

    const std::string& get_version() const noexcept { return m_version; }

    const Options& get_options() const noexcept { return m_options; }

    Options& get_options() { return m_options; }

    std::optional<AdbCfg> get_adb_cfg(const std::string& name) const
    {
        if (auto iter = m_adb_cfg.find(name); iter != m_adb_cfg.cend()) {
            return iter->second;
        }
        else {
            return std::nullopt;
        }
    }

    std::optional<std::string> get_package_name(const std::string& client_type) const
    {
        if (auto iter = m_package_name.find(client_type); iter != m_package_name.cend()) {
            return iter->second;
        }
        return std::nullopt;
    }

    void set_options(Options opt) noexcept { m_options = std::move(opt); }

protected:
    virtual bool parse(const json::value& json) override;

    std::string m_version;
    Options m_options;
    std::unordered_map<std::string, AdbCfg> m_adb_cfg;
    std::unordered_map<std::string, std::string> m_package_name;
};

inline static auto& Config = GeneralConfig::get_instance();
} // namespace asst
