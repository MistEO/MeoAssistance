#include "GeneralConfig.h"

#include "Utils/Logger.hpp"
#include <meojson/json.hpp>

void asst::GeneralConfig::set_connection_extras(const std::string& name, const json::object& extras)
{
    LogInfo << name << extras;

    m_adb_cfg[name].extras = extras;
}

bool asst::GeneralConfig::parse(const json::value& json)
{
    LogTraceFunction;

    m_version = json.at("version").as_string();

    {
        const json::value& options_json = json.at("options");
        m_options.task_delay = options_json.at("taskDelay").as_integer();
        m_options.sss_fight_screencap_interval = options_json.at("SSSFightScreencapInterval").as_integer();
        m_options.roguelike_fight_screencap_interval = options_json.at("RoguelikeFightScreencapInterval").as_integer();
        m_options.copilot_fight_screencap_interval = options_json.at("CopilotFightScreencapInterval").as_integer();
        m_options.control_delay_lower = options_json.at("controlDelayRange")[0].as_integer();
        m_options.control_delay_upper = options_json.at("controlDelayRange")[1].as_integer();
        // m_options.print_window = options_json.at("printWindow").as_boolean();
        m_options.adb_extra_swipe_dist = options_json.get("adbExtraSwipeDist", 100);
        m_options.adb_extra_swipe_duration = options_json.get("adbExtraSwipeDuration", -1);
        m_options.adb_swipe_duration_multiplier = options_json.get("adbSwipeDurationMultiplier", 10.0);
        m_options.adb_swipe_x_distance_multiplier = options_json.get("adbSwipeXDistanceMultiplier", 0.8);
        m_options.minitouch_extra_swipe_dist = options_json.get("minitouchExtraSwipeDist", 100);
        m_options.minitouch_extra_swipe_duration = options_json.get("minitouchExtraSwipeDuration", -1);
        m_options.minitouch_swipe_default_duration = options_json.get("minitouchSwipeDefaultDuration", 200);
        m_options.minitouch_swipe_extra_end_delay = options_json.get("minitouchSwipeExtraEndDelay", 150);
        m_options.swipe_with_pause_required_distance = options_json.get("swipeWithPauseRequiredDistance", 50);
        if (auto order = options_json.find<json::array>("minitouchProgramsOrder")) {
            m_options.minitouch_programs_order.clear();
            for (const auto& type : *order) {
                m_options.minitouch_programs_order.emplace_back(type.as_string());
            }
        }

        if (auto penguin_opt = options_json.find<json::object>("penguinReport")) {
            m_options.penguin_report.url = penguin_opt->get("url", std::string());
            m_options.penguin_report.timeout = penguin_opt->get("timeout", 10000);
            if (auto headers_opt = penguin_opt->find<json::object>("headers")) {
                m_options.penguin_report.headers.clear();
                for (const auto& [key, value] : *headers_opt) {
                    m_options.penguin_report.headers.emplace(key, value.as_string());
                }
            }
        }
        if (auto yituliu_opt = options_json.find<json::object>("yituliuReport")) {
            m_options.yituliu_report.drop_url = yituliu_opt->get("dropUrl", std::string());
            m_options.yituliu_report.recruit_url = yituliu_opt->get("recruitUrl", std::string());
            m_options.yituliu_report.timeout = yituliu_opt->get("timeout", 5000);
            if (auto headers_opt = yituliu_opt->find<json::object>("headers")) {
                m_options.yituliu_report.headers.clear();
                for (const auto& [key, value] : *headers_opt) {
                    m_options.yituliu_report.headers.emplace(key, value.as_string());
                }
            }
        }
        m_options.depot_export_template.ark_planner =
            options_json.get("depotExportTemplate", "arkPlanner", std::string());
        m_options.debug.clean_files_freq = options_json.get("debug", "cleanFilesFreq", 50);
        m_options.debug.max_debug_file_num = options_json.get("debug", "maxDebugFileNum", 100);
    }

    for (const auto& [client_type, package_name] : json.at("packageName").as_object()) {
        m_package_name[client_type] = package_name.as_string();
    }

    for (const auto& cfg_json : json.at("connection").as_array()) {
        std::string base_name = cfg_json.get("baseConfig", std::string());
        const AdbCfg& base_cfg = base_name.empty() ? AdbCfg() : m_adb_cfg.at(base_name);

        AdbCfg& adb = m_adb_cfg[cfg_json.at("configName").as_string()];
        adb.devices = cfg_json.get("devices", base_cfg.devices);
        adb.address_regex = cfg_json.get("addressRegex", base_cfg.address_regex);
        adb.connect = cfg_json.get("connect", base_cfg.connect);
        adb.display_id = cfg_json.get("displayId", base_cfg.display_id);
        adb.uuid = cfg_json.get("uuid", base_cfg.uuid);
        adb.click = cfg_json.get("click", base_cfg.click);
        adb.input = cfg_json.get("input", base_cfg.input);
        adb.swipe = cfg_json.get("swipe", base_cfg.swipe);
        adb.press_esc = cfg_json.get("pressEsc", base_cfg.press_esc);
        adb.display = cfg_json.get("display", base_cfg.display);
        adb.screencap_raw_with_gzip = cfg_json.get("screencapRawWithGzip", base_cfg.screencap_raw_with_gzip);
        adb.screencap_raw_by_nc = cfg_json.get("screencapRawByNC", base_cfg.screencap_raw_by_nc);
        adb.dcr_screencap_addr = cfg_json.get("screencapRawByDCR", base_cfg.dcr_screencap_addr);
        adb.nc_address = cfg_json.get("ncAddress", base_cfg.nc_address);
        adb.screencap_encode = cfg_json.get("screencapEncode", base_cfg.screencap_encode);
        adb.release = cfg_json.get("release", base_cfg.release);
        adb.start = cfg_json.get("start", base_cfg.start);
        adb.stop = cfg_json.get("stop", base_cfg.stop);
        adb.abilist = cfg_json.get("abilist", base_cfg.abilist);
        adb.version = cfg_json.get("version", base_cfg.version);
        adb.orientation = cfg_json.get("orientation", base_cfg.orientation);
        adb.push_minitouch = cfg_json.get("pushMinitouch", base_cfg.push_minitouch);
        adb.chmod_minitouch = cfg_json.get("chmodMinitouch", base_cfg.chmod_minitouch);
        adb.call_minitouch = cfg_json.get("callMinitouch", base_cfg.call_minitouch);
        adb.call_maatouch = cfg_json.get("callMaatouch", base_cfg.call_maatouch);
        adb.push_droidcast = cfg_json.get("pushDroidCastRaw", base_cfg.push_droidcast);
        adb.chmod_droidcast = cfg_json.get("chmodDroidCastRaw", base_cfg.chmod_droidcast);
        adb.call_droidcast = cfg_json.get("callDroidCastRaw", base_cfg.call_droidcast);
        adb.reforward_droidcast_port = cfg_json.get("reforwardDroidCastRawPort", base_cfg.reforward_droidcast_port);
        adb.back_to_home = cfg_json.get("back_to_home", base_cfg.back_to_home);
    }

    return true;
}
