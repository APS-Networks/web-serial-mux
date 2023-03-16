#pragma once

#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;


namespace smux::util {


struct file_descriptor
{
public:
    file_descriptor(fs::path const & path);
    file_descriptor(fs::path const & path, int flags);
    ~file_descriptor();

    file_descriptor(file_descriptor const &) = delete;
    auto operator=(file_descriptor const &) -> file_descriptor& = delete;

    file_descriptor(file_descriptor &&);
    auto operator=(file_descriptor &&) -> file_descriptor&;

    operator bool() const
    { return m_fd != 0; }

    operator int() const
    {  return m_fd; }

    auto path() const -> fs::path const &;
private:
    int m_fd;
    fs::path m_path;
};


template <typename Lockable>
auto make_unique_lock(Lockable & lockable)
{
    return std::unique_lock<Lockable>{lockable};
}

template <typename Lockable>
auto make_unique_lock(Lockable & lockable, std::defer_lock_t defer)
{
    return std::unique_lock<Lockable>{lockable, defer};
}

template <typename Lockable>
auto make_unique_lock(Lockable & lockable, std::try_to_lock_t try_to_lock)
{
    return std::unique_lock<Lockable>{lockable, try_to_lock};
}


template <typename Lockable>
auto make_unique_lock(Lockable & lockable, std::adopt_lock_t adopt_lock)
{
    return std::unique_lock<Lockable>{lockable, adopt_lock};
}


}