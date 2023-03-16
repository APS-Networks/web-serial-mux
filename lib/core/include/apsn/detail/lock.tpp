#pragma once

#include <apsn/error.hpp>

#include <chrono>
#include <ratio>


template <typename Rep, typename Period>
auto apsn::multiprocess_mutex::try_lock_for(
        std::chrono::duration<Rep, Period> const & duration)
    -> bool
{
    using namespace std::chrono;
    return try_lock_until(system_clock::now() + duration);
}


template <typename Clock, typename Duration>
auto apsn::multiprocess_mutex::try_lock_until(
        std::chrono::time_point<Clock, Duration> const & timepoint)
    -> bool
{
    using namespace std::chrono;

    auto secs = time_point_cast<seconds>(timepoint);
    auto ns = duration_cast<nanoseconds>(timepoint - secs);

    return this->timed_lock(secs.time_since_epoch().count(), ns.count());
}


namespace apsn::detail {

 
ERROR_CATEGORY_DECL(slock_file,              "slock::file")
ERROR_CATEGORY_DECL(slock_mtxattr_init,      "slock::mtxattr_init")
ERROR_CATEGORY_DECL(slock_mtxattr_setrobust, "slock::mtxattr_set_robust")
ERROR_CATEGORY_DECL(slock_mtxattr_settype,   "slock::mtxattr_set_type")
ERROR_CATEGORY_DECL(slock_mtxattr_destroy,   "slock::mtxattr_destroy")

ERROR_CATEGORY_DECL(slock_mtx_init,    "slock::mtx_init")
ERROR_CATEGORY_DECL(slock_mtx_unlock,  "slock::mtx_unlock")
ERROR_CATEGORY_DECL(slock_mtx_lock,    "slock::mtx_lock")
ERROR_CATEGORY_DECL(slock_mtx_consist, "slock::mtx_consistent")
ERROR_CATEGORY_DECL(slock_mtx_destroy, "slock::mtx_destroy")

ERROR_CATEGORY_DECL(slock_flock_lock,   "slock::flock_lock")
ERROR_CATEGORY_DECL(slock_flock_unlock, "slock::flock_unlock")

}
