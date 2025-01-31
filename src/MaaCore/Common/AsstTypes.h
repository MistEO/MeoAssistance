#pragma once

#include <array>
#include <climits>
#include <cmath>
#include <functional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "Utils/StringMisc.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace asst
{
constexpr double DoubleDiff = 1e-12;

static constexpr int WindowWidthDefault = 1280;
static constexpr int WindowHeightDefault = 720;

static constexpr double TemplThresholdDefault = 0.8;

enum class StaticOptionKey
{
    Invalid = 0,
    CpuOCR = 1, // use CPU to OCR, no value. It does not support switching after the resource is loaded.
    GpuOCR = 2, // use GPU to OCR, value is gpu_id int to string. It does not support switching after the resource
                // is loaded.
};

enum class InstanceOptionKey
{
    Invalid = 0,
    /* Deprecated */         // MinitouchEnabled = 1,
    TouchMode = 2,           // 触控模式设置， "minitouch" | "maatouch" | "adb"
    DeploymentWithPause = 3, // 自动战斗、肉鸽、保全 是否使用 暂停下干员， "0" | "1"
    AdbLiteEnabled = 4,      // 是否使用 AdbLite， "0" | "1"
    KillAdbOnExit = 5,       // 退出时是否杀掉 Adb 进程， "0" | "1"
};

enum class TouchMode
{
    Adb = 0,
    Minitouch = 1,
    Maatouch = 2,
    MacPlayTools = 3,
};

namespace ControlFeat
{
using Feat = int64_t;
constexpr Feat NONE = 0;
constexpr Feat SWIPE_WITH_PAUSE = 1 << 0;
constexpr Feat PRECISE_SWIPE = 1 << 1;

inline bool support(Feat feat, Feat target) noexcept
{
    return (feat & target) == target;
}
}

struct Point
{
    Point() = default;
    ~Point() = default;
    Point(const Point&) noexcept = default;
    Point(Point&&) noexcept = default;

    constexpr Point(int x, int y) :
        x(x),
        y(y)
    {
    }

    Point& operator=(const Point&) noexcept = default;
    Point& operator=(Point&&) noexcept = default;

    Point operator-() const noexcept { return { -x, -y }; }

    bool operator==(const Point& rhs) const noexcept { return x == rhs.x && y == rhs.y; }

    std::string to_string() const { return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

    explicit operator std::string() const { return to_string(); }

    static constexpr Point right() { return { 1, 0 }; }

    static constexpr Point down() { return { 0, 1 }; }

    static constexpr Point left() { return { -1, 0 }; }

    static constexpr Point up() { return { 0, -1 }; }

    static constexpr Point zero() { return { 0, 0 }; }

    bool empty() const noexcept { return x == 0 && y == 0; }

    // for std::map
    bool operator<(const Point& rhs) const noexcept { return x < rhs.x || (x == rhs.x && y < rhs.y); }

    int x = 0;
    int y = 0;

    // clang-format off
#define DEFINE_ASST_POINT_BINARY_OP_AND_ARG_ASSIGN(Op)                    \
    friend Point operator Op(const Point& lhs, const Point& rhs) noexcept \
    {                                                                     \
        return { lhs.x Op rhs.x, lhs.y Op rhs.y };                        \
    }                                                                     \
    friend Point& operator Op##=(Point& val, const Point& opd) noexcept   \
    {                                                                     \
        val.x Op## = opd.x;                                               \
        val.y Op## = opd.y;                                               \
        return val;                                                       \
    }
    // clang-format on

    DEFINE_ASST_POINT_BINARY_OP_AND_ARG_ASSIGN(+)
    DEFINE_ASST_POINT_BINARY_OP_AND_ARG_ASSIGN(-)
    DEFINE_ASST_POINT_BINARY_OP_AND_ARG_ASSIGN(*)

#undef DEFINE_ASST_POINT_BINARY_OP_AND_ARG_ASSIGN

    friend Point operator*(int scalar, const Point& value) noexcept { return { value.x * scalar, value.y * scalar }; }

    friend Point operator*(const Point& value, int scalar) noexcept { return { value.x * scalar, value.y * scalar }; }

    static int dot(const Point& lhs, const Point& rhs) noexcept { return (lhs.x * rhs.x) + (lhs.y * rhs.y); }

    static double distance(const Point& lhs, const Point& rhs) noexcept
    {
        return std::sqrt(std::pow(rhs.x - lhs.x, 2) + std::pow(rhs.y - lhs.y, 2));
    }

    double length() const noexcept { return std::sqrt(static_cast<double>(dot(*this, *this))); }
};

struct Rect
{
    Rect() = default;
    ~Rect() = default;
    Rect(const Rect&) noexcept = default;
    Rect(Rect&&) noexcept = default;

    Rect(int x, int y, int width, int height) :
        x(x),
        y(y),
        width(width),
        height(height)
    {
    }

    Rect operator*(double rhs) const { return { x, y, static_cast<int>(width * rhs), static_cast<int>(height * rhs) }; }

    int area() const noexcept { return width * height; }

    Rect center_zoom(double scale, int max_width = INT_MAX, int max_height = INT_MAX) const
    {
        const int half_width_scale = static_cast<int>(width * (1 - scale) / 2);
        const int half_height_scale = static_cast<int>(height * (1 - scale) / 2);
        Rect dst(
            x + half_width_scale,
            y + half_height_scale,
            static_cast<int>(width * scale),
            static_cast<int>(height * scale));
        if (dst.x < 0) {
            dst.x = 0;
        }
        if (dst.y < 0) {
            dst.y = 0;
        }
        if (dst.width + dst.x >= max_width) {
            dst.width = max_width - dst.x;
        }
        if (dst.height + dst.y >= max_height) {
            dst.height = max_height - dst.y;
        }
        return dst;
    }

    Rect& operator=(const Rect&) noexcept = default;
    Rect& operator=(Rect&&) noexcept = default;

    bool operator==(const Rect& rhs) const noexcept
    {
        return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
    }

    bool empty() const noexcept { return width == 0 || height == 0; }

    bool include(const Rect& rhs) const noexcept
    {
        return x <= rhs.x && y <= rhs.y && (x + width) >= (rhs.x + rhs.width) && (y + height) >= (rhs.y + rhs.height);
    }

    bool include(const Point& rhs) const noexcept
    {
        return x <= rhs.x && y <= rhs.y && (x + width) >= rhs.x && (y + height) >= rhs.y;
    }

    std::string to_string() const
    {
        return "[ " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(width) + ", " +
               std::to_string(height) + " ]";
    }

    explicit operator std::string() const { return to_string(); }

    Rect move(Rect move) const { return { x + move.x, y + move.y, move.width, move.height }; }

    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

template <typename To, typename From>
inline constexpr To make_rect(const From& rect)
{
    return To { rect.x, rect.y, rect.width, rect.height };
}

struct TextRect
{
    std::string to_string() const
    {
        return "{ " + text + ": " + rect.to_string() + ", score: " + std::to_string(score) + " }";
    }

    explicit operator std::string() const { return to_string(); }

    Rect rect;
    double score = 0.0;
    std::string text;
};

struct MatchRect
{
    std::string to_string() const { return "{ rect: " + rect.to_string() + ", score: " + std::to_string(score) + " }"; }

    explicit operator std::string() const { return to_string(); }

    Rect rect;
    double score = 0.0;
    std::string templ_name;
};
} // namespace asst

namespace std
{
template <>
struct hash<asst::Point>
{
    size_t operator()(const asst::Point& point) const noexcept
    {
        return std::hash<int>()(point.x) ^ std::hash<int>()(point.y);
    }
};

template <>
struct hash<asst::Rect>
{
    size_t operator()(const asst::Rect& rect) const noexcept
    {
        return std::hash<int>()(rect.x) ^ std::hash<int>()(rect.y) ^ std::hash<int>()(rect.width) ^
               std::hash<int>()(rect.height);
    }
};
}

namespace asst
{
template <typename CType>
struct ContainerHasher
{
    std::size_t operator()(const CType& container) const
    {
        using value_type = typename CType::value_type;
        std::size_t seed = container.size();
        for (const value_type& e : container) {
            seed ^= std::hash<value_type>()(e) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

enum class AlgorithmType
{
    Invalid = -1,
    JustReturn,
    MatchTemplate,
    OcrDetect,
};

inline AlgorithmType get_algorithm_type(std::string algorithm_str)
{
    utils::tolowers(algorithm_str);
    static const std::unordered_map<std::string_view, AlgorithmType> algorithm_map = {
        { "matchtemplate", AlgorithmType::MatchTemplate },
        { "justreturn", AlgorithmType::JustReturn },
        { "ocrdetect", AlgorithmType::OcrDetect },
    };
    if (algorithm_map.contains(algorithm_str)) {
        return algorithm_map.at(algorithm_str);
    }
    return AlgorithmType::Invalid;
}

inline std::string enum_to_string(AlgorithmType algo)
{
    static const std::unordered_map<AlgorithmType, std::string> algorithm_map = {
        { AlgorithmType::Invalid, "Invalid" },
        { AlgorithmType::JustReturn, "JustReturn" },
        { AlgorithmType::MatchTemplate, "MatchTemplate" },
        { AlgorithmType::OcrDetect, "OcrDetect" },
    };
    if (auto it = algorithm_map.find(algo); it != algorithm_map.end()) {
        return it->second;
    }
    return "Invalid";
}

enum class ProcessTaskAction
{
    Invalid = 0,
    BasicClick = 0x100,
    ClickSelf = BasicClick | 1, // 点击自身位置
    ClickRect = BasicClick | 2, // 点击指定区域
    // ClickRand = BasicClick | 4, // 点击随机区域
    DoNothing = 0x200, // 什么都不做
    Stop = 0x400,      // 停止当前Task
    Swipe = 0x1000,    // 滑动
    Input = 0x2000,    // 输入文本
};

inline ProcessTaskAction get_action_type(std::string action_str)
{
    utils::tolowers(action_str);
    static const std::unordered_map<std::string, ProcessTaskAction> action_map = {
        { "", ProcessTaskAction::DoNothing },          { "donothing", ProcessTaskAction::DoNothing },
        { "clickself", ProcessTaskAction::ClickSelf }, { "clickrect", ProcessTaskAction::ClickRect },
        { "stop", ProcessTaskAction::Stop },           { "swipe", ProcessTaskAction::Swipe },
        { "input", ProcessTaskAction::Input },
    };
    if (auto it = action_map.find(action_str); it != action_map.end()) {
        return it->second;
    }
    return ProcessTaskAction::Invalid;
}

inline std::string enum_to_string(ProcessTaskAction action)
{
    static const std::unordered_map<ProcessTaskAction, std::string> action_map = {
        { ProcessTaskAction::Invalid, "Invalid" },       { ProcessTaskAction::DoNothing, "DoNothing" },
        { ProcessTaskAction::BasicClick, "BasicClick" }, { ProcessTaskAction::ClickSelf, "ClickSelf" },
        { ProcessTaskAction::ClickRect, "ClickRect" },   { ProcessTaskAction::Stop, "Stop" },
        { ProcessTaskAction::Swipe, "Swipe" },           { ProcessTaskAction::Input, "Input" },
    };
    if (auto it = action_map.find(action); it != action_map.end()) {
        return it->second;
    }
    return "Invalid";
}

enum class TaskDerivedType
{
    Invalid = -1,
    Raw,      // 原始任务
    BaseTask, // BaseTask
    Implicit, // 隐式 TemplateTask
    Template, // 显式 TemplateTask
};

inline std::string enum_to_string(TaskDerivedType task_derived_type)
{
    static const std::unordered_map<TaskDerivedType, std::string> task_derived_type_map = {
        { TaskDerivedType::Raw, "Raw" },
        { TaskDerivedType::BaseTask, "BaseTask" },
        { TaskDerivedType::Implicit, "Implicit" },
        { TaskDerivedType::Template, "Template" },
    };
    if (auto it = task_derived_type_map.find(task_derived_type); it != task_derived_type_map.end()) {
        return it->second;
    }
    return "Invalid";
}

enum class MatchMethod
{
    Invalid = -1,
    Ccoeff = 0,
    RGBCount,
    HSVCount,
};

inline MatchMethod get_match_method(std::string method_str)
{
    utils::tolowers(method_str);
    static const std::unordered_map<std::string, MatchMethod> method_map = {
        { "ccoeff", MatchMethod::Ccoeff },
        { "rgbcount", MatchMethod::RGBCount },
        { "hsvcount", MatchMethod::HSVCount },
    };
    if (auto it = method_map.find(method_str); it != method_map.end()) {
        return it->second;
    }
    return MatchMethod::Invalid;
}

inline std::string enum_to_string(MatchMethod method)
{
    static const std::unordered_map<MatchMethod, std::string> method_map = {
        { MatchMethod::Invalid, "Invalid" },
        { MatchMethod::Ccoeff, "Ccoeff" },
        { MatchMethod::RGBCount, "RGBCount" },
        { MatchMethod::HSVCount, "HSVCount" },
    };
    if (auto it = method_map.find(method); it != method_map.end()) {
        return it->second;
    }
    return "Invalid";
}
} // namespace asst

namespace asst
{
using TaskList = std::vector<std::string>;

// 任务流程信息
struct TaskPipelineInfo
{
    constexpr TaskPipelineInfo() = default;
    constexpr virtual ~TaskPipelineInfo() = default;
    constexpr TaskPipelineInfo(const TaskPipelineInfo&) = default;
    constexpr TaskPipelineInfo(TaskPipelineInfo&&) noexcept = default;
    constexpr TaskPipelineInfo& operator=(const TaskPipelineInfo&) = default;
    constexpr TaskPipelineInfo& operator=(TaskPipelineInfo&&) noexcept = default;
    std::string name;       // 任务名
    TaskList next;          // 下一个可能的任务（列表）
    TaskList sub;           // 子任务（列表）
    TaskList on_error_next; // 任务出错之后要去执行什么
    TaskList exceeded_next; // 达到最多次数了之后，下一个可能的任务（列表）
    TaskList reduce_other_times; // 执行了该任务后，需要减少别的任务的执行次数。例如执行了吃理智药，
                                 // 则说明上一次点击蓝色开始行动按钮没生效，所以蓝色开始行动要-1
};

using TaskPipelinePtr = std::shared_ptr<TaskPipelineInfo>;
using TaskPipelineConstPtr = std::shared_ptr<const TaskPipelineInfo>;

// 任务继承信息
struct TaskDerivedInfo : public TaskPipelineInfo
{
    constexpr TaskDerivedInfo() = default;
    constexpr virtual ~TaskDerivedInfo() = default;
    constexpr TaskDerivedInfo(const TaskDerivedInfo&) = default;
    constexpr TaskDerivedInfo(TaskDerivedInfo&&) noexcept = default;
    constexpr TaskDerivedInfo& operator=(const TaskDerivedInfo&) = default;
    constexpr TaskDerivedInfo& operator=(TaskDerivedInfo&&) noexcept = default;
    TaskDerivedType type = TaskDerivedType::Raw; // 任务类型
    std::string base = "";                       // 继承自哪个任务（Raw 任务不需要）
    std::string prefix = "";                     // 需要添加的前缀（仅对 TemplateTask 生效）
};

using TaskDerivedPtr = std::shared_ptr<TaskDerivedInfo>;
using TaskDerivedConstPtr = std::shared_ptr<const TaskDerivedInfo>;

// 任务信息
struct TaskInfo : public TaskPipelineInfo
{
    constexpr TaskInfo() = default;
    constexpr virtual ~TaskInfo() = default;
    constexpr TaskInfo(const TaskInfo&) = default;
    constexpr TaskInfo(TaskInfo&&) noexcept = default;
    constexpr TaskInfo& operator=(const TaskInfo&) = default;
    constexpr TaskInfo& operator=(TaskInfo&&) noexcept = default;
    AlgorithmType algorithm = AlgorithmType::Invalid;      // 图像算法类型
    ProcessTaskAction action = ProcessTaskAction::Invalid; // 要进行的操作
    bool sub_error_ignored = false; // 子任务如果失败了，是否继续执行剩下的任务
    int max_times = INT_MAX;        // 任务最多执行多少次
    Rect specific_rect;             // 指定区域，目前仅针对ClickRect任务有用，会点这个区域
    int pre_delay = 0;              // 执行该任务前的延时
    int post_delay = 0;             // 执行该任务后的延时
    int retry_times = INT_MAX;      // 未找到图像时的重试次数
    Rect roi;                       // 要识别的区域，若为0则全图识别
    Rect rect_move;     // 识别结果移动：有些结果识别到的，和要点击的不是同一个位置。
                        // 即识别到了res，点击res + result_move的位置
    bool cache = false; // 是否使用缓存区域
    std::vector<int> special_params; // 某些任务会用到的特殊参数
    std::string input_text; // 输入任务的文字，目前希望仅针对 Input 任务有效， algorithm 为 JustReturn
};

using TaskPtr = std::shared_ptr<TaskInfo>;
using TaskConstPtr = std::shared_ptr<const TaskInfo>;

// 文字识别任务的信息
struct OcrTaskInfo : public TaskInfo
{
    constexpr OcrTaskInfo() = default;
    constexpr virtual ~OcrTaskInfo() override = default;
    constexpr OcrTaskInfo(const OcrTaskInfo&) = default;
    constexpr OcrTaskInfo(OcrTaskInfo&&) noexcept = default;
    constexpr OcrTaskInfo& operator=(const OcrTaskInfo&) = default;
    constexpr OcrTaskInfo& operator=(OcrTaskInfo&&) noexcept = default;
    std::vector<std::string> text; // 文字的容器，匹配到这里面任一个，就算匹配上了
    bool full_match = false;       // 是否需要全匹配，否则搜索到子串就算匹配上了
    bool is_ascii = false;         // 是否启用字符数字模型
    bool without_det = false;      // 是否不使用检测模型
    bool replace_full = false; // 匹配之后，是否将整个字符串replace（false是只替换match的部分）
    std::vector<std::pair<std::string, std::string>>
        replace_map;           // 部分文字容易识别错，字符串强制replace之后，再进行匹配
};

using OcrTaskPtr = std::shared_ptr<OcrTaskInfo>;
using OcrTaskConstPtr = std::shared_ptr<const OcrTaskInfo>;

// 图片匹配任务的信息
struct MatchTaskInfo : public TaskInfo
{
    constexpr MatchTaskInfo() = default;
    constexpr virtual ~MatchTaskInfo() override = default;
    constexpr MatchTaskInfo(const MatchTaskInfo&) = default;
    constexpr MatchTaskInfo(MatchTaskInfo&&) noexcept = default;
    constexpr MatchTaskInfo& operator=(const MatchTaskInfo&) = default;
    constexpr MatchTaskInfo& operator=(MatchTaskInfo&&) noexcept = default;
    using GrayRange = std::pair<int, int>;
    using ColorRange = std::pair<std::array<int, 3>, std::array<int, 3>>;
    using Range = std::variant<GrayRange, ColorRange>;
    using Ranges = std::vector<Range>;
    std::vector<std::string> templ_names; // 匹配模板图片文件名
    std::vector<double> templ_thresholds; // 模板匹配阈值
    std::vector<MatchMethod> methods;     // 匹配方法
    Ranges mask_ranges;      // 匹配掩码范围，TaskData 仅允许 array<int, 2>，但保留彩色掩码支持
    Ranges color_scales;     // 数色掩码范围
    bool color_close = true; // 数色时是否使用闭运算处理
};

using MatchTaskPtr = std::shared_ptr<MatchTaskInfo>;
using MatchTaskConstPtr = std::shared_ptr<const MatchTaskInfo>;

inline static const std::string UploadDataSource = "MaaAssistantArknights";
} // namespace asst

namespace asst
{
// steal from https://www.boost.org/doc/libs/1_85_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
// use boost if you prefer
template <typename T1, typename T2>
struct PairHash
{
    std::size_t operator()(const std::pair<T1, T2>& p) const
    {
        std::size_t hash1 = std::hash<T1> {}(p.first);
        std::size_t hash2 = std::hash<T2> {}(p.second);
        hash1 ^= hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2);

        hash1 ^= hash1 >> 32;
        hash1 *= 0xe9846af9b1a615d;
        hash1 ^= hash1 >> 32;
        hash1 *= 0xe9846af9b1a615d;
        hash1 ^= hash1 >> 28;

        return hash1;
    }
};
}
