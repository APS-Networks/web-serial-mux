#include "history.hpp"

#include <cassert>
#include <cstddef>
#include <list>
#include <string>


using smux::cli::history;


history::history()
    : m_commands{}
    , m_iter{std::end(m_commands)}
{
}


auto history::append(std::string command) -> void
{
    m_commands.emplace_back(std::move(command));
    m_iter = std::end(m_commands);
}


auto history::size() -> std::size_t
{
    return m_commands.size();
}


auto history::operator*() -> std::string
{
    if (m_commands.size() == 0) {
        assert(std::begin(m_commands) == std::end(m_commands));
        assert(m_iter == std::end(m_commands));
        assert(m_iter == std::begin(m_commands));
    }
    if (m_iter == std::end(m_commands)) {
        return "";
    }
    return *m_iter;
}


auto history::operator++() -> history&
{
    if (m_iter != std::end(m_commands)) {
        ++m_iter;
    }
    return *this;
}


auto history::operator--() -> history&
{
    if (m_iter != std::begin(m_commands)) {
        --m_iter;
    }
    return *this;
}


// auto history::operator++(int) -> history
// {
//     auto tmp = *this;
//     ++(*this);
//     return tmp;
// }   


// auto history::operator--(int) -> history
// {
//     auto tmp = *this;
//     --(*this);
//     return tmp;
// }

