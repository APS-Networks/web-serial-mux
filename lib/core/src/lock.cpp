#include "lock.hpp"

#include "error.hpp"
#include "result.hpp"
#include "logging.hpp"


#include <chrono>
#include <filesystem>
#include <memory>

#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>


struct apsn::multiprocess_mutex::impl
{
    int m_fd;
    bool m_recursive;
    flock m_flock;
    pthread_mutex_t m_mutex;
    std::size_t m_counter;
};



auto make_traced_error(
    int rv,
    std::error_category const& category,
    std::string message)
{
    auto err = std::error_code{ rv, category };
    apsn::log::trace("{}: {}", message, err.message());
    return err;  
}

#define STR1(x) #x
#define STR2(x) STR1(x)
#define SOURCE_LOCATION __FILE__ ":" STR2(__LINE__)
#define MAKE_TRACED_ERROR(rv, cat, message) \
        make_traced_error(rv, cat, SOURCE_LOCATION ": " message ": ");


auto initialise_lock_impl(std::filesystem::path const & path, bool reentrant)
        -> apsn::result<apsn::multiprocess_mutex::impl>
{
    using namespace apsn::detail;

    int fd = -255, rv = -1;

    /* TODO */ apsn::log::debug("Opening lock file ", path.native());
    fd = ::open( path.c_str(),
        O_RDWR    |
        O_CREAT   |
        O_CLOEXEC ,
        S_IRUSR   |
        S_IWUSR );
    if (fd < 0) {
        // return { std::nullopt, std::error_code{-fd, slock_file_category()} };
        auto err = MAKE_TRACED_ERROR(-fd, get_slock_file_category(), "Error opening file");
        return { std::nullopt, err };
    }

    flock flk{};

    /*  Types of lock 
        F_RDLCK: Read lock. Other processes can create a read lock on the
                 specified file segment, but cannot create a write lock.
        F_WRLCK: Write lock. No other process can create a read or write lock
                 within the specified segment of the file.
    */
    flk.l_type   = F_WRLCK;  /* Exclusive lock                   */
    flk.l_whence = SEEK_SET; /* l_start is relative to beginning */
    flk.l_start  = 0;        /* Offset from l_whence             */
    flk.l_len    = 0;        /* Indicates to end of file         */

    auto mtx_attr = ::pthread_mutexattr_t{};
    
    rv = ::pthread_mutexattr_init(&mtx_attr);
    if (rv != 0) {
        auto err = MAKE_TRACED_ERROR(rv, get_slock_mtx_init_category(), "Error init mutex");
        return { std::nullopt, err };
    }

    /* Prevents the mutex from being unusable if an owner terminates before
       releasing. So let's allow the mutex to be cleaned up. We're not expecting
       the owner to terminate, but since we control what this is going to be,
       might as well anyway. Default is PTHREAD_MUTEX_STALLED. */
    rv = ::pthread_mutexattr_setrobust(&mtx_attr, PTHREAD_MUTEX_ROBUST);
    if (rv != 0) {
        auto err = MAKE_TRACED_ERROR(rv, get_slock_mtxattr_setrobust_category(), "Error setting robust");
        return { std::nullopt, err };
    }

    if (reentrant) {
        /* Can return EINVAL */

        rv = ::pthread_mutexattr_settype(&mtx_attr, PTHREAD_MUTEX_RECURSIVE);
        if (rv != 0) {
            auto err = MAKE_TRACED_ERROR(rv, get_slock_mtxattr_settype_category(), "Error setting recursive");
            return { std::nullopt, err };
        }
    }
    else {
        /* Noop for now. We could however set PTHREAD_MUTEX_ERRORCHECK instead
           of having the default of PTHREAD_MUTEX_DEFAULT as it's less likely to
           invoke undefined behaviour. */
    }

    auto mtx = pthread_mutex_t{};

    /* Can return EAGAIN, ENOMEM or EPERM */
    rv = ::pthread_mutex_init(&mtx, &mtx_attr);
    if (rv != 0) {
        auto err = MAKE_TRACED_ERROR(rv, get_slock_mtx_init_category(), "Error init mutex");
        return { std::nullopt, err };
    }

    /* Can return EBUSY, EINVAL */
    rv = ::pthread_mutexattr_destroy(&mtx_attr);
    if (rv != 0) {
        auto err = MAKE_TRACED_ERROR(rv, get_slock_mtxattr_destroy_category(), "Error destroy attr");
        return { std::nullopt, err };
    }

    auto impl = apsn::multiprocess_mutex::impl{};
    impl.m_fd = fd;
    impl.m_flock = flk;
    impl.m_recursive = reentrant;
    impl.m_mutex = mtx;

    /* TODO */ apsn::log::debug("Shared lock initialised");

    return { impl, std::error_code{} };
}


apsn::multiprocess_mutex::multiprocess_mutex(std::filesystem::path path, bool reentrant)
    : pimpl{std::make_unique<impl>()}
{
    auto && [init, err] = initialise_lock_impl(path, reentrant);

    if (err) {
        /* TODO */ apsn::log::fatal("Error creating shared lock ", path.native(), 
                ". source: ", err.category().name(), ", message: ", err.message());
        throw std::system_error{err};
    }
    
    /*  Assign to the dereferenced impl pointer with the value of the 
        dereferenced optional impl */
    *pimpl = *init;
}


enum class lock_procedure
{
    lock,
    try_lock
};




auto lock_impl(apsn::multiprocess_mutex::impl * lock) -> std::error_code
{
    using namespace apsn::detail;

    int rv = 0;

    if ((rv = ::pthread_mutex_lock(&lock->m_mutex)) != 0) {
        
        if (rv == EOWNERDEAD) {
            /* TODO */ apsn::log::trace("Previous mutex holder terminated (EOWNERDEAD)");
            /*  This will happen if the mutex is in an inconsistent state,
                caused by an owner terminating before releasing the lock. 
                This result will happen if the mutex is set up to be robust,
                which we do. */
            if ((rv = ::pthread_mutex_consistent(&lock->m_mutex)) != 0) {
                /* Returns EINVAL if not robust, or is not inconsistent.
                    Do nothing for now -- hopefully EOWNERDEAD will never be
                    returned if neither of aforementioned conditions hold
                    true. */
                return MAKE_TRACED_ERROR( rv, get_slock_mtx_consist_category(),
                    "Error making mutex consistent");
            }
            if ((rv = ::pthread_mutex_lock(&lock->m_mutex)) != 0) {
                return MAKE_TRACED_ERROR( rv, get_slock_mtx_lock_category(), 
                    "Error locking after making consistent" );
            }
        }
        else {
            return MAKE_TRACED_ERROR( rv, get_slock_mtx_lock_category(),
                "Error locking" );
        }
    }

    auto err = std::error_code{};

    if (lock->m_counter == 0) {
        lock->m_flock.l_type = F_WRLCK;

        if (::fcntl(lock->m_fd, F_SETLK, &lock->m_flock) == -1) {
            err = MAKE_TRACED_ERROR( errno, get_slock_flock_lock_category(),
                "Could not lock file");
        }
    }

    if (err) {
        if ((rv = ::pthread_mutex_unlock(&lock->m_mutex)) != 0) {
            /* Sets errno to EPERM, EINVAL or EAGAIN. But this should never 
            happen. */
            err = MAKE_TRACED_ERROR( rv, get_slock_mtx_unlock_category(),
                "Could not unlock mutex" );
        }
    }

    if (lock->m_recursive) {
        lock->m_counter++;
    }

    return err;
}


auto try_lock_impl(apsn::multiprocess_mutex::impl * lock) -> std::error_code
{
    using namespace apsn::detail;

    int rv = 0;

    if ((rv = ::pthread_mutex_trylock(&lock->m_mutex)) != 0) {
        
        if (rv == EBUSY || rv == EPERM) {
            return MAKE_TRACED_ERROR( rv, get_slock_mtx_lock_category(), 
                "Unable to acquire lock");
        }

        if (rv == EOWNERDEAD) {
            /* TODO */ apsn::log::trace("Previous mutex holder terminated (EOWNERDEAD)");
            /*  This will happen if the mutex is in an inconsistent state,
                caused by an owner terminating before releasing the lock. 
                This result will happen if the mutex is set up to be robust,
                which we do. */
            if ((rv = ::pthread_mutex_consistent(&lock->m_mutex)) != 0) {
                /* Returns EINVAL if not robust, or is not inconsistent.
                    Do nothing for now -- hopefully EOWNERDEAD will never be
                    returned if neither of aforementioned conditions hold
                    true. */
                return MAKE_TRACED_ERROR( rv, get_slock_mtx_consist_category(),
                    "Error making mutex consistent");
            }
            if ((rv = ::pthread_mutex_trylock(&lock->m_mutex)) != 0) {
                if (rv == EBUSY || rv == EPERM) {
                    return MAKE_TRACED_ERROR( rv, get_slock_mtx_lock_category(), 
                        "Unable to acquire lock");
                }
                return MAKE_TRACED_ERROR( rv, get_slock_mtx_lock_category(), 
                    "Error locking after making consistent" );
            }
        }
        else {
            return MAKE_TRACED_ERROR( rv, get_slock_mtx_lock_category(),
                "Error locking" );
        }
    }

    auto err = std::error_code{};

    if (lock->m_counter == 0) {
        lock->m_flock.l_type = F_WRLCK;

        if (::fcntl(lock->m_fd, F_SETLK, &lock->m_flock) == -1) {
            err = MAKE_TRACED_ERROR( errno, get_slock_flock_lock_category(),
                    "Could not lock file");
        }
    }

    if (err) {
        if ((rv = ::pthread_mutex_unlock(&lock->m_mutex)) != 0) {
            /* Sets errno to EPERM, EINVAL or EAGAIN. But this should never 
            happen. */
            err = MAKE_TRACED_ERROR( rv, get_slock_mtx_unlock_category(),
                    "Could not unlock mutex" );
        }
    }

    if (lock->m_recursive) {
        lock->m_counter++;
    }
    return err;
}




auto unlock_impl(apsn::multiprocess_mutex::impl * lock) -> std::error_code
{
    using namespace apsn::detail;

    int rv = 0;

    if (lock->m_counter == 0) {
        lock->m_flock.l_type = F_UNLCK;
        if (::fcntl(lock->m_fd, F_SETLK, &lock->m_flock) == -1) {
            /* Sets errno to EACCES or EAGAIN */
            return MAKE_TRACED_ERROR( errno, get_slock_flock_unlock_category(),
                    "Error unlocking flock");
        }
    }

    if ((rv = ::pthread_mutex_unlock(&lock->m_mutex)) != 0) {
        /*  EPERM, EINVAL or EAGAIN. But this should never happen. */
        return MAKE_TRACED_ERROR( errno, get_slock_mtx_unlock_category(),
                "Error unlocking mutex");
    }

    if (lock->m_recursive) {
        if (lock->m_counter == 0) {
            LOG_TRACE("Counter is already 0!");
        }
        lock->m_counter--;
    }

    return {};
}


apsn::multiprocess_mutex::~multiprocess_mutex()
{
    using namespace apsn::detail;
    int rv;
    auto err = std::error_code{};
    if ((rv = ::close(pimpl->m_fd)) != 0) {
        err = MAKE_TRACED_ERROR( rv, get_slock_file_category(), "Error closing file");
    }
    /*  Following std::mutex semantics, behaviour is undefined if lock is held
        when destruction occurs */
    if ((rv = ::pthread_mutex_destroy(&pimpl->m_mutex)) != 0) {
        err = MAKE_TRACED_ERROR( rv, get_slock_mtx_destroy_category(),
                "Error destroying mutex");
    }
    if (err) {
        /* TODO */ apsn::log::debug("Error closing shared lock: ", err.message());
    }
}


auto apsn::multiprocess_mutex::lock() -> void
{
    auto err = lock_impl(pimpl.get());
    if (err) {
        throw std::system_error{err};
    }
    LOG_TRACE("Success creating lock, lock count: ",
            pimpl->m_counter);
}

auto apsn::multiprocess_mutex::try_lock() noexcept -> bool
{
    auto err = try_lock_impl(pimpl.get());
    return !err;
}

auto apsn::multiprocess_mutex::unlock() -> void
{
    auto err = unlock_impl(pimpl.get());
    if (err) {
        throw std::system_error{err};
    }
    LOG_TRACE("Successfully unlocked shared lock, lock count: ",
            pimpl->m_counter);
}

#define NANOSLEEP_NO_REMAINDER NULL

auto apsn::multiprocess_mutex::timed_lock(std::time_t seconds_since_epoch,
        long nanoseconds)
    -> bool
{
    using namespace apsn::detail;

    auto d = std::chrono::seconds{seconds_since_epoch} +
            std::chrono::nanoseconds{nanoseconds};
    
    auto tp = std::chrono::system_clock::time_point{
        std::chrono::duration_cast<std::chrono::system_clock::duration>(d)};
    auto tt = std::chrono::system_clock::to_time_t(tp);

    auto buf = std::stringstream{};
    buf << std::put_time(std::localtime(&tt), "%Y-%m-%d %X");
    LOG_TRACE("Try locking until ", buf.str());

    auto ts = timespec {seconds_since_epoch, nanoseconds};
    auto rv = 0;

    if ((rv = pthread_mutex_timedlock(&pimpl->m_mutex, &ts)) != 0) {
        MAKE_TRACED_ERROR(rv, get_slock_mtx_lock_category(), "Could not acquire timed lock");
        return false;
    }

    timespec current_time, wait_for;
    wait_for.tv_nsec = 10000;

    auto err = std::error_code{};

    for (;;) {
        ::clock_gettime(CLOCK_REALTIME, &current_time);
        if (current_time.tv_sec > ts.tv_sec &&
                current_time.tv_nsec > ts.tv_sec) {
            err = MAKE_TRACED_ERROR(ETIMEDOUT, get_slock_flock_lock_category(),
                "Timeout while waiting for flock");
            break;
        }

        if (::fcntl(pimpl->m_fd, F_SETLK, &pimpl->m_flock) == 0) {
            break;
        }

        if (::nanosleep(&wait_for, NANOSLEEP_NO_REMAINDER) == -1) {
            err = MAKE_TRACED_ERROR(errno, get_slock_flock_lock_category(), 
                "Timed lock Sleep interrupted");
        }
    }

    if (!err) {
        if (pimpl->m_recursive) {
            pimpl->m_counter++;
        }
    }
    else {
        if ((rv = pthread_mutex_unlock(&pimpl->m_mutex)) != 0) {
            err = MAKE_TRACED_ERROR( errno, get_slock_mtx_unlock_category(),
                "Error unlocking mutex");
        }
    }

    return !err;
}