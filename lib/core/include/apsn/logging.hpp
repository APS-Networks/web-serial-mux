#pragma once

#include <fmt/core.h>

#include <atomic>
#include <iosfwd>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>


namespace apsn::log {

enum class level
{
    fatal = 2, // jounrnald "crit" message
    error = 3,
    warn  = 4, 
    info  = 6,
    debug = 7,
    trace = 8,
};

auto to_string(level lvl) -> std::string;
auto level_from_string(std::string const & lvl) -> level;
auto operator<<(std::ostream & lhs, level const & rhs) -> std::ostream&;
auto operator>>(std::istream& lhs, level& rhs) -> std::istream&;

class logger
{
public:
    logger(std::string name, level threshold = level::info);

    virtual ~logger() = default;

    
    template <typename ... Args>
    auto fatal(fmt::format_string<Args...> format, Args && ... args) -> void
    { return log(level::fatal, format, std::forward<Args>(args)...); }
    
    template <typename ... Args>
    auto error(fmt::format_string<Args...> format, Args && ... args) -> void
    { return log(level::error, format, std::forward<Args>(args)...); }
    
    template <typename ... Args>
    auto warn(fmt::format_string<Args...> format, Args && ... args) -> void
    { return log(level::warn, format, std::forward<Args>(args)...); }
    
    template <typename ... Args>
    auto info(fmt::format_string<Args...> format, Args && ... args) -> void
    { return log(level::info, format, std::forward<Args>(args)...); }
    
    template <typename ... Args>
    auto debug(fmt::format_string<Args...> format, Args && ... args) -> void
    { return log(level::debug, format, std::forward<Args>(args)...); }
    
    template <typename ... Args>
    auto trace(fmt::format_string<Args...> format, Args && ... args) -> void
    { return log(level::trace, format, std::forward<Args>(args)...); }


    auto fatal(std::string const & msg) -> void;
    auto error(std::string const & msg) -> void;
    auto warn(std::string const & msg) -> void;
    auto info(std::string const & msg) -> void;
    auto debug(std::string const & msg) -> void;
    auto trace(std::string const & msg) -> void;

    template <typename ... Args>
    auto log(level msg_level, 
            fmt::format_string<Args...> format,
            Args && ... args)
        -> void
    {
        if (msg_level <= m_threshold) {
            auto lock = std::unique_lock<std::mutex>(m_mtx);
            auto result = fmt::format(format, std::forward<Args>(args)...);
            do_log(msg_level, std::move(result));
        }
    }

    auto log(level msg_level, std::string const & msg)
        -> void
    {
        if (msg_level <= m_threshold) {
            auto lock = std::unique_lock<std::mutex>(m_mtx);
            do_log(msg_level, msg);
        }
    }

    auto threshold(level threshold)
    { m_threshold = threshold; }

    auto threshold() const -> log::level
    { return m_threshold; }

    auto name() const -> std::string
    { return m_name; }

    auto name(std::string name) -> void
    { m_name = name; }

    virtual auto do_log(level msg_level, std::string const & message) 
        -> void = 0;

protected:
    std::string m_name;

private:
    level m_threshold;
    std::mutex m_mtx;
};



class coloured_cli_logger : public logger
{
public:
    coloured_cli_logger(std::string name, level threshold = level::info);
    coloured_cli_logger(std::ostream& out,
            std::string name,
            level threshold = level::info);
    ~coloured_cli_logger();
private:
    auto do_log(level msg_level, std::string const & message)
        -> void final override;

    std::ostream & m_out;
};



// auto set_global_logger(logger * instance) -> void;
auto get_logger() -> logger&;
inline
auto set_threshold(log::level threshold) -> void
{
    get_logger().threshold(threshold);
}

namespace detail {

auto default_logger() -> std::shared_ptr<logger>;
auto get_logger_holder() -> std::shared_ptr<logger>&;
auto set_global_logger(std::shared_ptr<logger> instance) -> void;

// auto default_logger() -> logger*;
// auto get_logger_holder() -> std::atomic<logger*>&;

}

template <typename T, typename ... Args>
auto make_logger(Args && ... args)
{
    auto instance = std::make_shared<T>(std::forward<Args>(args)...);
    detail::set_global_logger(instance);
}


template <typename ... Args>
auto fatal(fmt::format_string<Args...> format, Args && ... args) -> void
{ 
    return get_logger().fatal(format, std::forward<Args>(args)...);
}

template <typename ... Args>
auto error(fmt::format_string<Args...> format, Args && ... args) -> void
{ 
    return get_logger().error(format, std::forward<Args>(args)...);
}

template <typename ... Args>
auto warn(fmt::format_string<Args...> format, Args && ... args) -> void
{ 
    return get_logger().warn(format, std::forward<Args>(args)...);
}

template <typename ... Args>
auto info(fmt::format_string<Args...> format, Args && ... args) -> void
{ 
    return get_logger().info(format, std::forward<Args>(args)...);
}

template <typename ... Args>
auto debug(fmt::format_string<Args...> format, Args && ... args) -> void
{ 
    return get_logger().debug(format, std::forward<Args>(args)...);
}

template <typename ... Args>
auto trace(fmt::format_string<Args...> format, Args && ... args) -> void
{ 
    return get_logger().trace(format, std::forward<Args>(args)...);
}

/* For pybind11 and none fomrat string messages */
auto fatal(std::string const & msg) -> void;
auto error(std::string const & msg) -> void;
auto warn(std::string const & msg) -> void;
auto info(std::string const & msg) -> void;
auto debug(std::string const & msg) -> void;
auto trace(std::string const & msg) -> void;

}


#define LOG_TRACE(...) apsn::log::trace ( __FILE__, ":", __LINE__, ": ", __VA_ARGS__)

