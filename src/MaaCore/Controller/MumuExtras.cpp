#include "MumuExtras.h"

#if ASST_WITH_EMULATOR_EXTRAS

#include "Utils/Logger.hpp"
#include "Utils/NoWarningCV.h"

namespace asst
{
MumuExtras::~MumuExtras()
{
    uninit();
}

bool MumuExtras::init(const std::filesystem::path& mumu_path, int mumu_inst_index)
{
    bool same = mumu_path == mumu_path_ && mumu_inst_index == mumu_inst_index_;

    if (same && inited_) {
        return true;
    }

    mumu_path_ = mumu_path;
    mumu_inst_index_ = mumu_inst_index;

    inited_ = load_mumu_library() && connect_mumu() && init_screencap();

    return inited_;
}

void MumuExtras::set_package_name(const std::string& package_name)
{
    if (package_name.empty()) {
        package_name_ = kDefaultPackage;
    }
    else {
        package_name_ = package_name;
    }
}

bool MumuExtras::reload()
{
    inited_ = load_mumu_library() && connect_mumu() && init_screencap();
    LogInfo << "Reload MumuExtras: " << VAR(inited_);
    return inited_;
}

void MumuExtras::uninit()
{
    inited_ = false;
    disconnect_mumu();
}

std::optional<cv::Mat> MumuExtras::screencap()
{
    if (!capture_display_func_) {
        LogError << "capture_display_func_ is null";
        return std::nullopt;
    }

    int display_id = get_display_id();
    int ret = capture_display_func_(
        mumu_handle_,
        display_id,
        static_cast<int>(display_buffer_.size()),
        &display_width_,
        &display_height_,
        display_buffer_.data());

    if (ret) {
        // Try reloading once before giving up.
        if (!reload()) {
            LogError << "Failed to capture display and failed to reload. " << VAR(ret) << VAR(mumu_handle_)
                     << VAR(display_id) << VAR(display_buffer_.size()) << VAR(display_width_) << VAR(display_height_);
            return std::nullopt;
        }
        if (ret) {
            LogError << "Failed to capture display, but reload before retrying capture was successful. " << VAR(ret)
                     << VAR(mumu_handle_) << VAR(display_id) << VAR(display_buffer_.size()) << VAR(display_width_)
                     << VAR(display_height_);
            return std::nullopt;
        }
    }

    cv::Mat raw(display_height_, display_width_, CV_8UC4, display_buffer_.data());
    cv::Mat bgr;
    cv::cvtColor(raw, bgr, cv::COLOR_RGBA2BGR);
    cv::Mat dst;
    cv::flip(bgr, dst, 0);

    return dst;
}

bool MumuExtras::load_mumu_library()
{
    auto lib_path = mumu_path_ / "shell/sdk/external_renderer_ipc";

    if (!load_library(lib_path)) {
        LogError << "Failed to load library" << VAR(lib_path);
        return false;
    }

    connect_func_ = get_function<decltype(nemu_connect)>(kConnectFuncName);
    if (!connect_func_) {
        LogError << "Failed to get function" << VAR(kConnectFuncName);
        return false;
    }

    disconnect_func_ = get_function<decltype(nemu_disconnect)>(kDisconnectFuncName);
    if (!disconnect_func_) {
        LogError << "Failed to get function" << VAR(kDisconnectFuncName);
        return false;
    }

    get_display_id_func_ = get_function<decltype(nemu_get_display_id)>(kGetDisplayIdFuncName);
    if (!get_display_id_func_) {
        // 旧版本 mumu 没这个函数, 兼容一下
        LogWarn << "Failed to get function, please update your MuMu Player" << VAR(kGetDisplayIdFuncName);
        // return false;
    }

    capture_display_func_ = get_function<decltype(nemu_capture_display)>(kCaptureDisplayFuncName);
    if (!capture_display_func_) {
        LogError << "Failed to get function" << VAR(kCaptureDisplayFuncName);
        return false;
    }

    input_text_func_ = get_function<decltype(nemu_input_text)>(kInputTextFuncName);
    if (!input_text_func_) {
        LogError << "Failed to get function" << VAR(kInputTextFuncName);
        return false;
    }

    input_event_touch_down_func_ = get_function<decltype(nemu_input_event_touch_down)>(kInputEventTouchDownFuncName);
    if (!input_event_touch_down_func_) {
        LogError << "Failed to get function" << VAR(kInputEventTouchDownFuncName);
        return false;
    }

    input_event_touch_up_func_ = get_function<decltype(nemu_input_event_touch_up)>(kInputEventTouchUpFuncName);
    if (!input_event_touch_up_func_) {
        LogError << "Failed to get function" << VAR(kInputEventTouchUpFuncName);
        return false;
    }

    input_event_key_down_func_ = get_function<decltype(nemu_input_event_key_down)>(kInputEventKeyDownFuncName);
    if (!input_event_key_down_func_) {
        LogError << "Failed to get function" << VAR(kInputEventKeyDownFuncName);
        return false;
    }

    input_event_key_up_func_ = get_function<decltype(nemu_input_event_key_up)>(kInputEventKeyUpFuncName);
    if (!input_event_key_up_func_) {
        LogError << "Failed to get function" << VAR(kInputEventKeyUpFuncName);
        return false;
    }

    return true;
}

bool MumuExtras::connect_mumu()
{
    LogInfo << VAR(mumu_path_) << VAR(mumu_inst_index_);

    if (!connect_func_) {
        LogError << "connect_func_ is null";
        return false;
    }

    std::u16string u16path = mumu_path_.u16string();
    std::wstring wpath(std::make_move_iterator(u16path.begin()), std::make_move_iterator(u16path.end()));

    mumu_handle_ = connect_func_(wpath.c_str(), mumu_inst_index_);

    if (mumu_handle_ == 0) {
        LogError << "Failed to connect mumu" << VAR(mumu_path_) << VAR(mumu_inst_index_);
        return false;
    }

    return true;
}

bool MumuExtras::init_screencap()
{
    if (!capture_display_func_) {
        LogError << "capture_display_func_ is null";
        return false;
    }

    int display_id = get_display_id();
    LogInfo << "Get display id" << VAR(display_id);

    int ret = capture_display_func_(mumu_handle_, display_id, 0, &display_width_, &display_height_, nullptr);

    // mumu 的文档给错了，这里 0 才是成功
    if (ret) {
        LogError << "Failed to capture display" << VAR(ret) << VAR(mumu_handle_) << VAR(display_id);
        return false;
    }

    display_buffer_.resize(display_width_ * display_height_ * 4);

    LogDebug << VAR(display_width_) << VAR(display_height_) << VAR(display_buffer_.size());
    return true;
}

void MumuExtras::disconnect_mumu()
{
    LogInfo << VAR(mumu_handle_);

    if (mumu_handle_ != 0) {
        disconnect_func_(mumu_handle_);
    }
}

int MumuExtras::get_display_id()
{
    if (!get_display_id_func_) {
        LogError << "get_display_id_func_ is null, please update your MuMu Player";
        return 0;
    }

    int id = get_display_id_func_(mumu_handle_, package_name_.c_str(), 0);
    if (id < 0) {
        LogWarn << "Failed to get display id" << VAR(id) << VAR(package_name_);
        return 0;
    }

    return id;
}
} // namespace asst

#endif
