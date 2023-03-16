#include "utility.hpp"

#include <fcntl.h>
#include <unistd.h>

using smux::util::file_descriptor;


file_descriptor::file_descriptor(
        fs::path const & path,
        int flags)
    : m_fd{::open(path.c_str(), flags)}
{
}


file_descriptor::file_descriptor(fs::path const & path)
    : m_fd{::open(path.c_str(), 0)}
{
}


file_descriptor::file_descriptor(file_descriptor && other)
    : m_fd{std::move(other.m_fd)}
    , m_path{std::move(other.m_path)}
{
}


file_descriptor::~file_descriptor()
{
    if (*this) {
        ::close(m_fd);
    }
}


auto file_descriptor::operator=(file_descriptor && other) -> file_descriptor&
{
    m_fd = std::move(other.m_fd);
    m_path = std::move(other.m_path);
    other.m_fd = 0;
    other.m_path.clear();
    return *this;
}
