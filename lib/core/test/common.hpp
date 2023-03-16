#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace smux::test {
/* Class to collect results of an ostream write */
class streambuf : public std::streambuf
{
public:
    auto xsputn(char const * s, std::streamsize n)
        -> std::streamsize override
    {
        m_data.insert(m_data.size(), s, n);
        return n;
    }
    auto overflow(int c) -> int override
    {
        m_data.push_back(static_cast<char>(c));
        return 1;
    }
    auto str() const -> std::string
    {
        return m_data;
    }
    auto clear() -> void
    {
        return m_data.clear();
    }
private:
    std::string m_data;
};


class ostream_delay
{
    using clock_type = std::chrono::system_clock;
    using duration_type = typename clock_type::duration;
public:
    template <typename Rep, typename Period>
    ostream_delay(std::chrono::duration<Rep, Period> delay_by)
        : m_delay_by{std::chrono::duration_cast<duration_type>(delay_by)}
    {
    }
    auto delay() const -> void
    {
        std::this_thread::sleep_for(m_delay_by);
    }
private:
    duration_type m_delay_by;
};

inline
auto operator<<(std::ostream& lhs, ostream_delay const & rhs) -> std::ostream&
{
    rhs.delay();
    return lhs;
}

}