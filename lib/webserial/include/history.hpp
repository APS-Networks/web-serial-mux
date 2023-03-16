#pragma once

#include <cstddef>
#include <list>
#include <string>

namespace smux::cli {

class history {
public:
    history();
    history(history const & other) 
        : m_commands{other.m_commands}
        , m_iter{std::end(m_commands)}
    {
    }
    history(history && other) = default;

    auto operator=(history const & other) -> history & 
    {
        if (this == &other) {
            return *this;
        }
        m_commands = other.m_commands;
        m_iter = std::end(m_commands);
        return *this;
    }
    auto operator=(history && other) -> history & = default;
    auto append(std::string command) -> void;
    auto size() -> std::size_t;
    auto operator*() -> std::string;
    auto operator++() -> history&;
    auto operator--() -> history&;
    // auto operator++(int) -> history;
    // auto operator--(int) -> history;
private:
    std::list<std::string> m_commands;
    // typename std::list<std::string>::const_reverse_iterator m_iter;
    typename std::list<std::string>::const_iterator m_iter;
};


}