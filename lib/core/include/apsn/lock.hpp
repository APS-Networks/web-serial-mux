#pragma once

#include <chrono>
#include <filesystem>
#include <memory>

namespace apsn {

/**
 * @brief Mutex for co-operative locking across processes
 * 
 * Many aspects of platform managment require exclusive access to some system
 * wide resource. An example of such a resource are the `i2c` buses on which
 * the system EEPROM, various CPLDs and ultimately, the SFP EEPROMs, reside.
 * 
 * Each SFP has the same `i2c` management address, and are provided via 
 * multiple stages of multiplexers that must be configured to make the 
 * chain of segments available to the host system.  It would be possible for one
 * process to set the multiplexers to reach one optic for a read operation in
 * the time between another process setting up the tree for a write and
 * performing the write. The consequence of this is that the write operation is
 * applied to the wrong optic.
 *
 * The best case scenario, should something like this happen, is that read
 * operations fail because the addresses they are trying to reach have dropped
 * off the bus. The worst case scenario is a write going to somewhere it
 * shouldn't, potentially causing system instability.
 * 
 * The lock itself meets several of the C++ named requirements for a mutex, and
 * can be used alongside other threading primitives.
 * 
 * Example:
 * 
 * \code {.cpp}

        #include <apsn/lock.hpp>

        #include <atomic>
        #include <chrono>
        #include <condition_variable>
        #include <mutex>
        #include <thread>

        using namespace std::chrono_literals;

        auto main() -> int {
            auto mtx = apsn::multiprocess_mutex{"lock.lck", false};
            auto cv = std::condition_variable_any{};
            auto end = std::atomic<bool>{false};

            auto thread = std::thread{[&](){
                std::this_thread::sleep_for(2s);
                auto lock = std::unique_lock<apsn::multiprocess_mutex>{mtx};
                end = true;
                lock.unlock();
                cv.notify_one();
            }};

            auto lck = std::unique_lock<apsn::multiprocess_mutex>{mtx};
            cv.wait(lck, [&](){ return end.load(); });

            thread.join();
        }

 * \endcode
 * 
 */
class multiprocess_mutex
{
public:
    /**
     * @brief Construct a new multiprocess mtx object
     * 
     * If the file on which to place the lock does not exist, one will be 
     * created.
     * 
     * @param path Path to a file on which to place the lock
     * @param reentrant If true, the lock can be locked repeatedly by the thread
     *                  currently holding the lock.
     */
    multiprocess_mutex(std::filesystem::path path, bool reentrant);

    /**
     * @brief Construct a new multiprocess mtx object
     * 
     * If the file on which to place the lock does not exist, one will be 
     * created.
     * 
     * The resultant lock object will not be re-entrant. This will mean any
     * subsequent calls to `lock` after the mutex has been locked will deadlock.
     * 
     * @param path Path to a file on which to place the lock
     */
    multiprocess_mutex(std::filesystem::path path)
        : multiprocess_mutex{path, false}
    {}

    multiprocess_mutex(multiprocess_mutex const&) = delete;
    multiprocess_mutex(multiprocess_mutex&&) = delete;

    auto operator=(multiprocess_mutex const &) = delete;
    auto operator=(multiprocess_mutex &&) = delete;

    ~multiprocess_mutex();


    /**
     * @brief Locks mutex, blocks if not available
     * 
     * The locking process itself involves the following:
     * 
     * 1. Attempt to lock the shared lock's mutex. If the mutex is in an
     *    inconsistent state because it has been locked, but not unlocked by a
     *    thread that no longer exists, it will be made consistent, and a lock
     *    will be tried again. If any of this fails, return;
     * 
     * 2. If we haven't previously been locked (accommodating re-entrant locks),
     *    attempt to issue a write lock to the lock file. If this fails, unlock
     *    the mutex and return.
     * 
     * Named requirements: BasicLockable
     */
    auto lock() -> void; 


    /**
     * @brief Unlocks the mutex
     * 
     * It is undefined behaviour to call unlock on a mutex which is already 
     * locked.
     * 
     * Named requirements: BasicLockable
     */
    auto unlock() -> void;


    /**
     * @brief Attempt to lock the mutex and return immediately
     * 
     *  Named requirements: Lockable
     * 
     * @return true If mutex was successfully locked
     * @return false If mutex was unable to be locked
     */
    auto try_lock() noexcept -> bool;


    /**
     * @brief Attempt to lock the mutex within timeout
     * 
     * If the lock is acquired within the timeout, this will immediately return.
     * 
     * Named requirements: TimedLockable 
     * 
     * @tparam Rep Tick/counter type
     * @tparam Period Interval between ticks
     * @param timeout Length of time to try locking
     * @return true Mutex was able to be locked within the timeout
     * @return false Mutex was unable to be locked within the timeout
     */
    template <typename Rep, typename Period>
    auto try_lock_for(std::chrono::duration<Rep, Period> const & timeout) -> bool;


    /**
     * @brief Attempt to lock until a point in the future.
     * 
     * If the lock is acquired before the time point, this will immediately
     * return.
     * 
     * Named requirements: TimedLockable 
     * 
     * @tparam Clock Clock on which the time point is measured
     * @tparam Duration Duration type
     * @param timepoint Time in the future with which to lock the mutex by.
     * @return true if mutex was able to be locked before time point
     * @return false if mutex was unable to be locked before time point
     */
    template <typename Clock, typename Duration = typename Clock::duration>
    auto try_lock_until(
            std::chrono::time_point<Clock, Duration> const & timepoint) -> bool;

    struct impl;

private:
    std::unique_ptr<impl> pimpl;

    auto timed_lock(std::time_t seconds_since_epoc, long nanoseconds) -> bool;

};

}

#include <apsn/detail/lock.tpp>
