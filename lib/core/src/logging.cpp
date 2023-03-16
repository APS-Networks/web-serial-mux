#include "logging.hpp"

#include <apsn/ansi.hpp>
#include <apsn/fmt.hpp>

#include <fmt/chrono.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>


using apsn::log::coloured_cli_logger;
using apsn::log::logger;
using apsn::log::level;


logger::logger(std::string name, level threshold)
    : m_name{name}
    , m_threshold{threshold}
{}


auto logger::fatal(std::string const & msg) -> void
{
    this->log(level::fatal, msg);
}


auto logger::error(std::string const & msg) -> void
{
    this->log(level::error, msg);
}


auto logger::warn(std::string const & msg) -> void
{
    this->log(level::warn, msg);
}


auto logger::info(std::string const & msg) -> void
{
    this->log(level::info, msg);
}


auto logger::debug(std::string const & msg) -> void
{
    this->log(level::debug, msg);
}


auto logger::trace(std::string const & msg) -> void
{
    this->log(level::trace, msg);
}




auto apsn::log::to_string(level lvl) -> std::string
{
    auto mapped = std::map<level, std::string> {
            { level::fatal, "fatal" },
            { level::error, "error" },
            { level::warn,  "warn"  },
            { level::info,  "info"  },
            { level::debug, "debug" },
            { level::trace, "trace" },
    };
    return mapped.at(lvl);
}


auto apsn::log::level_from_string(std::string const & lvl) -> level
{
    static auto log_level_map = std::map<std::string, level>{
        {"trace", level::trace },
        {"debug", level::debug },
        {"info",  level::info  },
        {"warn",  level::warn  },
        {"error", level::error },
        {"fatal", level::fatal }
    };
    return log_level_map.at(lvl);
}


auto apsn::log::operator>>(std::istream& lhs, level& rhs) -> std::istream&
{
    auto token = std::string{};
    lhs >> token;
    if (token == "fatal") { rhs = level::fatal; }
    else if (token == "error") { rhs = level::error; }
    else if (token == "warn") { rhs = level::warn; }
    else if (token == "info") { rhs = level::info; }
    else if (token == "debug") { rhs = level::debug; }
    else if (token == "trace") { rhs = level::trace; }
    else {
        lhs.setstate(std::ios::failbit);
    }
    return lhs;
}


auto apsn::log::operator<<(std::ostream & lhs, level const & rhs)
    -> std::ostream&
{
     return lhs << to_string(rhs);
}


coloured_cli_logger::coloured_cli_logger(
        std::string name,
        level threshold)
    : logger{name, threshold}
    , m_out{std::cout}
{}


coloured_cli_logger::coloured_cli_logger(
        std::ostream& out,
        std::string name,
        level threshold)
    : logger{name, threshold}
    , m_out{out}
{}


coloured_cli_logger::~coloured_cli_logger()
{
}


struct thread_number
{
    static std::size_t counter;

    thread_number()
        : m_number{counter++}
    {}

    operator std::size_t()
    {
        return m_number;
    }

    std::size_t m_number;
};

std::size_t thread_number::counter = 0ull;

auto get_thread_number() -> std::size_t
{
    static auto map = std::map<std::thread::id, thread_number>{};
    return map[std::this_thread::get_id()];
}


auto coloured_cli_logger::do_log(level msg_level, std::string const & message)
    -> void
{
    auto time = std::time(nullptr);
    auto tm = fmt::localtime(time);
    auto date = fmt::format("[{:%Y-%m-%d %H:%M:%S}]", tm);
    auto str = std::string{};
    str.reserve(128);

    auto it = std::back_inserter(str);

    fmt::format_to(it, "{}{}{} ", ansi::italic, date, ansi::reset);
    if (threshold() == level::trace) {
        fmt::format_to(it, "[Thread-{}] ", get_thread_number());
    }
    fmt::format_to(it, "{}{}: ", ansi::bold, m_name);

    switch (msg_level) {
    case level::fatal: fmt::format_to(it, "{}fatal: ", ansi::red); break;
    case level::error: fmt::format_to(it, "{}error: ", ansi::red); break;
    case level::warn:  fmt::format_to(it, "{}warn:  ", ansi::yellow);  break;
    case level::info:  fmt::format_to(it, "{}info:  ", ansi::green);  break;
    case level::debug: fmt::format_to(it, "{}debug: ", ansi::cyan); break;
    case level::trace: fmt::format_to(it, "{}trace: ", ansi::magenta); break;
    }
    fmt::format_to(it, "{}{}\n", ansi::reset, message);
    
    m_out << str;
}


namespace apsn::log::detail {

auto default_logger() -> std::shared_ptr<logger>
{
    static auto logger = std::make_shared<coloured_cli_logger>(
        "default", level::info
    );
    return logger;
}


auto get_logger_holder() -> std::shared_ptr<logger>&
{
    static auto holder = std::shared_ptr<logger>{default_logger()};
    return holder;
}

auto set_global_logger(std::shared_ptr<logger> instance) -> void
{
    get_logger_holder().swap(instance);
}

}



auto apsn::log::get_logger() -> logger&
{
    return *detail::get_logger_holder();
}



auto apsn::log::fatal(std::string const & msg) -> void
{
    return get_logger().fatal("{}", msg);
}


auto apsn::log::error(std::string const & msg) -> void
{
    return get_logger().error("{}", msg);
}


auto apsn::log::warn(std::string const & msg) -> void
{
    return get_logger().warn("{}", msg);
}


auto apsn::log::info(std::string const & msg) -> void
{
    return get_logger().info("{}", msg);
}


auto apsn::log::debug(std::string const & msg) -> void
{
    return get_logger().debug("{}", msg);
}


auto apsn::log::trace(std::string const & msg) -> void
{
    return get_logger().trace("{}", msg);
}