#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>

#include "Common/AsstTypes.h"
#include "Common/AsstVersion.h"
#include "Locale.hpp"
#include "Meta.hpp"
#include "Platform.hpp"
#include "Ranges.hpp"
#include "SingletonHolder.hpp"
#include "Time.hpp"
#include "WorkingDir.hpp"

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif

namespace asst
{
template <typename Stream, typename T>
concept has_stream_insertion_operator = requires { std::declval<Stream&>() << std::declval<T>(); };
template <typename T>
concept enum_could_to_string = requires { asst::enum_to_string(std::declval<T>()); };

// is_reference_wrapper
template <typename T>
struct is_reference_wrapper : std::false_type
{
};

template <typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type
{
};

template <typename T>
struct is_reference_wrapper<std::reference_wrapper<T>&> : std::true_type
{
};

template <typename T>
struct is_reference_wrapper<std::reference_wrapper<T>&&> : std::true_type
{
};

template <typename T>
inline constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

// from_reference_wrapper
template <typename T>
struct from_reference_wrapper
{
    typedef T type;
};

template <typename T>
struct from_reference_wrapper<std::reference_wrapper<T>>
{
    typedef typename from_reference_wrapper<T&>::type type;
};

template <typename T>
struct from_reference_wrapper<std::reference_wrapper<T>&>
{
    typedef typename from_reference_wrapper<T&>::type type;
};

template <typename T>
struct from_reference_wrapper<std::reference_wrapper<T>&&>
{
    typedef typename from_reference_wrapper<T&>::type type;
};

template <typename T>
using from_reference_wrapper_t = typename from_reference_wrapper<T>::type;

// to_reference_wrapper
template <typename T>
struct to_reference_wrapper
{
    typedef T type;
};

template <typename T>
struct to_reference_wrapper<T&>
{
    typedef std::reference_wrapper<T> type;
};

template <typename T>
struct to_reference_wrapper<std::reference_wrapper<T>>
{
    typedef typename to_reference_wrapper<T&>::type type;
};

template <typename T>
struct to_reference_wrapper<std::reference_wrapper<T>&>
{
    typedef typename to_reference_wrapper<T&>::type type;
};

template <typename T>
struct to_reference_wrapper<std::reference_wrapper<T>&&>
{
    typedef typename to_reference_wrapper<T&>::type type;
};

template <typename T>
using to_reference_wrapper_t = typename to_reference_wrapper<T>::type;

template <typename T>
struct remove_cvref
{
    typedef std::remove_cvref_t<from_reference_wrapper_t<std::remove_cvref_t<T>>> type;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename T>
constexpr from_reference_wrapper_t<T> convert_reference_wrapper(T&& x)
{
    return x;
}

namespace detail
{
class scope_slice
{
public:
    using id = int;

    scope_slice() = default;

    std::string push(id& i)
    {
        i = 0;
        if (auto iter = std::find_if(m_state.rbegin(), m_state.rend(), [](int e) { return e != -1; });
            iter != m_state.rend()) {
            i = *iter + 1;
        }

        std::string result;
        result.reserve(m_state.size() * 3);
        for (auto e : m_state) {
            result += (e == -1 ? " " : "│");
        }

        m_state.push_back(i);
        result += "┌";

        return result;
    }

    std::string pop(id i)
    {
        std::string result;
        result.reserve(m_state.size() * 3);
        std::string sym = "│";
        std::string emp = " ";
        for (auto& e : m_state) {
            if (i != -1 && e == i) {
                result += "└";
                e = -1;
                sym = "┼";
                emp = "─";
            }
            else {
                result += (e == -1 ? emp : sym);
            }
        }

        while (!m_state.empty() && m_state.back() == -1) {
            m_state.pop_back();
        }
        return result;
    }

    std::string next()
    {
        while (!m_state.empty() && m_state.back() == -1) {
            m_state.pop_back();
        }

        std::string result;
        result.reserve(m_state.size() * 3);
        auto lhs = m_state.end();
        auto rhs = m_state.end();
        auto iter = m_state.begin();
        //      | <- rhs
        // ------
        // | <- lhs
        for (; iter != m_state.end(); ++iter) {
            if (*iter != -1) {
                result += "│";
                continue;
            }

            lhs = iter;
            result += "┌";
            // if (iter != m_state.end())
            ++iter;

            for (; iter != m_state.end(); ++iter) {
                if (*iter == -1) {
                    result += "─";
                }
                else {
                    break;
                }
            }
            rhs = iter;
            result += "┘";

            if (iter != m_state.end()) {
                ++iter;
            }

            for (; iter != m_state.end(); ++iter) {
                result += (*iter == -1 ? " " : "│");
            }
            break;
        }

        if (lhs != m_state.end()) {
            if (rhs == m_state.end()) {
                m_state.erase(lhs, rhs);
            }
            else {
                std::swap(*lhs, *rhs);
            }
        }

        return result;
    }

private:
    std::vector<id> m_state {};
};
} // namespace detail

class console_ostream
{
    std::reference_wrapper<std::ostream> m_ofs;

public:
    console_ostream(console_ostream&&) = default;
    console_ostream(const console_ostream&) = default;
    console_ostream& operator=(console_ostream&&) = default;
    console_ostream& operator=(const console_ostream&) = default;

    console_ostream(std::ostream& stream) :
        m_ofs(stream)
    {
    }

    console_ostream(std::reference_wrapper<std::ostream> stream) :
        m_ofs(stream)
    {
    }

    template <typename T>
    requires has_stream_insertion_operator<std::ostream, T>
    console_ostream& operator<<(T&& v)
    {
#ifdef _WIN32
        if constexpr (std::convertible_to<T, std::string_view>) {
            asst::utils::utf8_scope scope(m_ofs.get());
            m_ofs.get() << std::forward<T>(v);
        }
        else {
            m_ofs.get() << std::forward<T>(v);
        }
#else
        m_ofs.get() << std::forward<T>(v);
#endif
        return *this;
    }

    console_ostream& operator<<(std::ostream& (*pf)(std::ostream&))
    {
        m_ofs.get() << pf;
        return *this;
    }
};

template <typename... Args>
class ostreams
{
    std::tuple<Args...> m_ofss;

public:
    ostreams(ostreams&&) = default;
    ostreams(const ostreams&) = default;
    ostreams& operator=(ostreams&&) = default;
    ostreams& operator=(const ostreams&) = default;

    ostreams(Args&&... args) :
        m_ofss(std::forward<Args>(args)...)
    {
    }

    template <typename T>
    requires has_stream_insertion_operator<std::ostream, T>
    ostreams& operator<<(T&& x)
    {
        streams_put(m_ofss, x, std::index_sequence_for<Args...> {});
        return *this;
    }

    template <typename Tuple, typename T, size_t... Is>
    static void streams_put(Tuple& t, T&& x, std::index_sequence<Is...>)
    {
        ((convert_reference_wrapper(std::get<Is>(t)) << std::forward<T>(x)), ...);
    }

    ostreams& operator<<(std::ostream& (*pf)(std::ostream&))
    {
        streams_put(m_ofss, pf, std::index_sequence_for<Args...> {});
        return *this;
    }

    template <typename Tuple, size_t... Is>
    static void streams_put(Tuple& t, std::ostream& (*pf)(std::ostream&), std::index_sequence<Is...>)
    {
        ((convert_reference_wrapper(std::get<Is>(t)) << pf), ...);
    }
};

template <typename... Args>
ostreams(Args&&...) -> ostreams<to_reference_wrapper_t<Args>...>;

class Logger : public SingletonHolder<Logger>
{
public:
    struct separator
    {
        constexpr separator() = default;
        constexpr separator(const separator&) = default;
        constexpr separator(separator&&) noexcept = default;

        constexpr explicit separator(std::string_view s) noexcept :
            str(s)
        {
        }

        constexpr separator& operator=(const separator&) = default;
        constexpr separator& operator=(separator&&) noexcept = default;

        constexpr separator& operator=(std::string_view s) noexcept
        {
            str = s;
            return *this;
        }

        static const separator none;
        static const separator space;
        static const separator tab;
        static const separator newline;
        static const separator comma;

        std::string_view str;
    };

    struct level
    {
        constexpr level(const level&) = default;
        constexpr level(level&&) noexcept = default;

        constexpr explicit level(std::string_view s) noexcept :
            str(s)
        {
        }

        constexpr level& operator=(const level&) = default;
        constexpr level& operator=(level&&) noexcept = default;

        constexpr level& operator=(std::string_view s) noexcept
        {
            str = s;
            return *this;
        }

        bool enabled = true;

        bool is_enabled() const { return enabled; }

        void set_enabled(bool enable) { enabled = enable; }

        static level debug;
        static level trace;
        static level info;
        static level warn;
        static level error;

        std::string_view str;
    };

    template <typename stream_t>
    class LogStream
    {
    public:
        template <typename T>
        LogStream& operator<<(T&& arg)
        {
            if constexpr (std::same_as<separator, remove_cvref_t<T>>) {
                m_sep = std::forward<T>(arg);
            }
            else {
                // 如果是 level，则不输出 separator
                if constexpr (!std::same_as<Logger::level, remove_cvref_t<T>>) {
                    stream_put(m_ofs, m_sep.str);
                }
                stream_put(m_ofs, std::forward<T>(arg));
            }
            return *this;
        }

        template <typename _stream_t = stream_t>
        LogStream(std::mutex& mtx, _stream_t&& ofs, Logger::level lv) :
            m_trace_lock(mtx),
            m_ofs(ofs)
        {
            *this << lv;
        }

        template <typename _stream_t = stream_t, typename... Args>
        LogStream(std::mutex& mtx, _stream_t&& ofs, Logger::level lv, Args&&... buff) :
            m_trace_lock(mtx),
            m_ofs(ofs)
        {
            ((*this << lv) << ... << std::forward<Args>(buff));
        }

        template <typename _stream_t = stream_t, typename... Args>
        LogStream(std::unique_lock<std::mutex>&& lock, _stream_t&& ofs, Logger::level lv, Args&&... buff) :
            m_trace_lock(std::move(lock)),
            m_ofs(ofs)
        {
            ((*this << lv) << ... << std::forward<Args>(buff));
        }

        LogStream(LogStream&&) = delete;
        LogStream(const LogStream&) = delete;
        LogStream& operator=(LogStream&&) = delete;
        LogStream& operator=(const LogStream&) = delete;

        ~LogStream() { m_ofs << std::endl; }

    private:
        template <typename Stream, typename T>
        static Stream& stream_put(Stream& s, T&& v)
        {
            if constexpr (std::same_as<std::filesystem::path, remove_cvref_t<T>>) {
                s << utils::path_to_utf8_string(std::forward<T>(v));
            }
            else if constexpr (std::same_as<Logger::level, remove_cvref_t<T>>) {
                constexpr int buff_len = 128;
                char buff[buff_len] = { 0 };
#ifdef _WIN32
#ifdef _MSC_VER
                sprintf_s(
                    buff,
                    buff_len,
#else  // ! _MSC_VER
                sprintf(
                    buff,
#endif // END _MSC_VER
                    "[%s][%s][Px%x][Tx%4.4lx]",
                    asst::utils::get_format_time().c_str(),
                    v.str.data(),
                    m_pid,
                    m_tid);
#else  // ! _WIN32
                sprintf(
                    buff,
                    "[%s][%s][Px%x][Tx%4.4hx]",
                    asst::utils::get_format_time().c_str(),
                    v.str.data(),
                    m_pid,
                    m_tid);
#endif // END _WIN32
                s << buff;
            }
            else if constexpr (std::is_enum_v<T> && enum_could_to_string<T>) {
                s << asst::enum_to_string(std::forward<T>(v));
            }
            else if constexpr (has_stream_insertion_operator<Stream, T>) {
                s << std::forward<T>(v);
            }
            else if constexpr (std::constructible_from<std::string, T>) {
                s << std::string(std::forward<T>(v));
            }
            else if constexpr (ranges::input_range<T>) {
                s << "[";
                std::string_view comma_space {};
                for (const auto& elem : std::forward<T>(v)) {
                    s << comma_space;
                    stream_put(s, elem);
                    comma_space = ", ";
                }
                s << "]";
            }
            else {
                ASST_STATIC_ASSERT_FALSE(
                    "unsupported type, one of the following expected\n"
                    "\t1. those can be converted to string;\n"
                    "\t2. those can be inserted to stream with operator<< directly;\n"
                    "\t3. container or nested container containing 1. 2. or 3. and iterable with "
                    "range-based for",
                    Stream,
                    T);
            }
            return s;
        }

        separator m_sep = separator::space;
        std::unique_lock<std::mutex> m_trace_lock;
        stream_t m_ofs;

        inline static thread_local const auto m_pid =
#ifdef _WIN32
            _getpid();
#else
            ::getpid();
#endif

        inline static thread_local const auto m_tid =
#ifdef _WIN32
            ::GetCurrentThreadId();
#else
            static_cast<unsigned short>(std::hash<std::thread::id> {}(std::this_thread::get_id()));
#endif
    };

    // template <typename stream_t>
    // LogStream(std::mutex&, stream_t&) -> LogStream<stream_t&>;

    // template <typename stream_t>
    // LogStream(std::mutex&, stream_t&&) -> LogStream<stream_t>;

    template <typename stream_t, typename... Args>
    LogStream(std::mutex&, stream_t&, Args&&...) -> LogStream<stream_t&>;

    template <typename stream_t, typename... Args>
    LogStream(std::mutex&, stream_t&&, Args&&...) -> LogStream<stream_t>;

    template <typename stream_t, typename... Args>
    LogStream(std::unique_lock<std::mutex>&&, stream_t&&, Args&&...) -> LogStream<stream_t>;

public:
    virtual ~Logger() override
    {
        std::unique_lock<std::mutex> m_trace_lock(m_trace_mutex);
        if (m_ofs.is_open()) {
            m_ofs.close();
        }
    }

    // static bool set_directory(const std::filesystem::path& dir)
    // {
    //     if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
    //         return false;
    //     }
    //     m_directory = dir;

    //     return true;
    // }

    template <typename T>
    auto operator<<(T&& arg)
    {
        if (!m_ofs || !m_ofs.is_open()) {
            m_ofs = std::ofstream(m_log_path, std::ios::out | std::ios::app);
        }
        rotate_check();
        if constexpr (std::same_as<level, remove_cvref_t<T>>) {
#ifdef ASST_DEBUG
            return LogStream(m_trace_mutex, ostreams { console_ostream(std::cout), m_ofs }, arg);
#else
            return LogStream(m_trace_mutex, m_ofs, arg);
#endif
        }
        else {
#ifdef ASST_DEBUG
            return LogStream(m_trace_mutex, ostreams { console_ostream(std::cout), m_ofs }, level::trace, arg);
#else
            return LogStream(m_trace_mutex, m_ofs, level::trace, arg);
#endif
        }
    }

#ifdef ASST_DEBUG
#define LOGGER_FUNC_WITH_LEVEL(lv)                                                     \
    template <typename... Args>                                                        \
    inline void lv(Args&&... args)                                                     \
    {                                                                                  \
        std::unique_lock lock { m_trace_mutex };                                       \
        log(std::move(lock), level::lv, m_scopes.next(), std::forward<Args>(args)...); \
    }
#else
#define LOGGER_FUNC_WITH_LEVEL(lv)                   \
    template <typename... Args>                      \
    inline void lv(Args&&... args)                   \
    {                                                \
        log(level::lv, std::forward<Args>(args)...); \
    }
#endif

    LOGGER_FUNC_WITH_LEVEL(trace)
    LOGGER_FUNC_WITH_LEVEL(info)
    LOGGER_FUNC_WITH_LEVEL(warn)
    LOGGER_FUNC_WITH_LEVEL(error)

    template <typename... Args>
    inline void debug([[maybe_unused]] Args&&... args)
    {
#ifdef ASST_DEBUG
        constexpr bool need_log = true;
#else
        static const bool need_log = std::filesystem::exists("DEBUG.txt");
#endif
        if (need_log) {
            std::unique_lock lock { m_trace_mutex };
            log(std::move(lock), level::debug, m_scopes.next(), std::forward<Args>(args)...);
        }
    }

#undef LOGGER_FUNC_WITH_LEVEL

    template <typename... Args>
    inline int push(Args&&... args)
    {
        int id = -1;
        std::unique_lock lock { m_trace_mutex };
        log(std::move(lock), level::trace, m_scopes.push(id), std::forward<Args>(args)...);
        return id;
    }

    template <typename... Args>
    inline void pop(int id, Args&&... args)
    {
        std::unique_lock lock { m_trace_mutex };
        log(std::move(lock), level::trace, m_scopes.pop(id), std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void log(level lv, Args&&... args)
    {
        if (lv.is_enabled()) {
            ((*this << lv) << ... << std::forward<Args>(args));
        }
    }

    template <typename... Args>
    inline void log(std::unique_lock<std::mutex>&& lock, level lv, Args&&... args)
    {
        if (!lv.is_enabled()) {
            return;
        }
        if (!m_ofs || !m_ofs.is_open()) {
            m_ofs = std::ofstream(m_log_path, std::ios::out | std::ios::app);
        }
        rotate_check(lock);
        (LogStream(
             std::move(lock),
#ifdef ASST_DEBUG
             ostreams { console_ostream(std::cout), m_ofs },
#else
             m_ofs,
#endif
             lv)
         << ... << std::forward<Args>(args));
    }

    void rotate_check()
    {
        if (!m_ofs || !m_ofs.is_open() || m_ofs.tellp() <= MaxLogSize) {
            return;
        }
        std::unique_lock<std::mutex> trace_lock(m_trace_mutex);
        rotate_check(trace_lock);
    }

    void rotate_check([[maybe_unused]] std::unique_lock<std::mutex>& lock)
    {
        if (!m_ofs || !m_ofs.is_open() || m_ofs.tellp() <= MaxLogSize) {
            return;
        }
        rotate();
    }

private:
    friend class SingletonHolder<Logger>;

    Logger() :
        m_directory(UserDir.get())
    {
        std::filesystem::create_directories(m_log_path.parent_path());
        log_init_info();
    }

    void rotate()
    {
        try {
            if (!std::filesystem::exists(m_log_path) || !std::filesystem::is_regular_file(m_log_path)) {
                return;
            }
            m_ofs.close();
            std::filesystem::rename(m_log_path, m_log_bak_path);
            m_ofs = std::ofstream(m_log_path, std::ios::out | std::ios::app);
        }
        catch (std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }
        catch (...) {
        }
    }

    void log_init_info()
    {
        trace("-----------------------------");
        trace("MaaCore Process Start");
        trace("Version", asst::Version);
        trace("Built at", __DATE__, __TIME__);
        trace("User Dir", m_directory);
        trace("-----------------------------");
    }

    detail::scope_slice m_scopes;

    std::filesystem::path m_directory;

    std::filesystem::path m_log_path = m_directory / "debug" / "asst.log";
    std::filesystem::path m_log_bak_path = m_directory / "debug" / "asst.bak.log";
    std::mutex m_trace_mutex;
    std::ofstream m_ofs;
    const long long MaxLogSize = 64LL * 1024 * 1024;
};

inline constexpr Logger::separator Logger::separator::none;
inline constexpr Logger::separator Logger::separator::space(" ");
inline constexpr Logger::separator Logger::separator::tab("\t");
inline constexpr Logger::separator Logger::separator::newline("\n");
inline constexpr Logger::separator Logger::separator::comma(",");

inline Logger::level Logger::level::debug("DBG");
inline Logger::level Logger::level::trace("TRC");
inline Logger::level Logger::level::info("INF");
inline Logger::level Logger::level::warn("WRN");
inline Logger::level Logger::level::error("ERR");

class LoggerAux
{
public:
    explicit LoggerAux(std::string_view func_name) :
        m_func_name(func_name),
        m_start_time(std::chrono::steady_clock::now())
    {
#ifdef ASST_DEBUG
        m_id = Logger::get_instance().push
#else
        Logger::get_instance().trace
#endif
               (m_func_name, "| enter");
    }

    ~LoggerAux()
    {
        const auto duration = std::chrono::steady_clock::now() - m_start_time;
#ifdef ASST_DEBUG
        Logger::get_instance().pop(
            m_id,
#else
        Logger::get_instance().trace(
#endif
            m_func_name,
            "| leave,",
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count(),
            "ms");
    }

    LoggerAux(const LoggerAux&) = default;
    LoggerAux(LoggerAux&&) = default;
    LoggerAux& operator=(const LoggerAux&) = default;
    LoggerAux& operator=(LoggerAux&&) = default;

private:
    std::string m_func_name;
    std::chrono::time_point<std::chrono::steady_clock> m_start_time;
    int m_id [[maybe_unused]] = -1;
};

#define _Cat_(a, b) a##b
#define _Cat(a, b) _Cat_(a, b)
#define _CatVarNameWithLine(Var) _Cat(Var, __LINE__)

#define Log asst::Logger::get_instance()
#define LogDebug Log << asst::Logger::level::debug
#define LogTrace Log << asst::Logger::level::trace
#define LogInfo Log << asst::Logger::level::info
#define LogWarn Log << asst::Logger::level::warn
#define LogError Log << asst::Logger::level::error

#define LogTraceScope LoggerAux _CatVarNameWithLine(_func_aux_)

#ifndef _MSC_VER
inline constexpr std::string_view summarize_pretty_function(std::string_view pf) // can be consteval?
{
    // unable to handle something like std::function<void(void)> gen_func()
    const auto paren = pf.find_first_of('(');
    if (paren != std::string_view::npos) {
        pf.remove_suffix(pf.size() - paren);
    }
    const auto space = pf.find_last_of(' ');
    if (space != std::string_view::npos) {
        pf.remove_prefix(space + 1);
    }
    return pf;
}

// TODO: use std::source_location
#define LogTraceFunction                                       \
    LogTraceScope                                              \
    {                                                          \
        ::asst::summarize_pretty_function(__PRETTY_FUNCTION__) \
    }
#else
#define LogTraceFunction LogTraceScope(__FUNCTION__)
#endif

#define VAR_RAW(x) "[" << #x << "=" << (x) << "] "
#define VAR(x) Logger::separator::none << VAR_RAW(x) << Logger::separator::space
#define VAR_VOIDP_RAW(x) "[" << #x << "=" << reinterpret_cast<void*>(x) << "] "
#define VAR_VOIDP(x) Logger::separator::none << VAR_VOIDP_RAW(x) << Logger::separator::space

#define LogTraceFunctionWithArgs // how to do this?, like LogTraceScope(__FUNCTION__,
                                 // __FUNCTION_ALL_ARGS__)
} // namespace asst
