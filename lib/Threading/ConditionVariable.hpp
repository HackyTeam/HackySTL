#pragma once

/// @file ConditionVariable.hpp
/// @brief Condition-variable waiting and notification primitives.
/// @details Coordinates mutex-protected predicates with blocking, timed, and
/// notifying operations for thread synchronization.

#include "Lock.hpp"
#include "Time.hpp"

namespace hsd
{
    namespace cv_detail
    {
        /// @brief Largest relative timeout, in milliseconds, a native wait accepts.
        inline constexpr u32 max_timeout_ms = 0xFFFF'FFFEu;

        /// @brief Converts the time left until `abs_time` into a relative timeout.
        /// @details Native Windows waits take a relative timeout in milliseconds.
        /// The result rounds up so a wait never ends before its deadline, is
        /// zero for deadlines already past, and is capped at `max_timeout_ms`.
        /// @param abs_time Absolute deadline.
        /// @param now Current time on the same clock as `abs_time`.
        /// @return Whole milliseconds remaining until `abs_time`.
        static inline u32 timeout_ms(
            const precise_clock& abs_time, const precise_clock& now)
        {
            const timespec _left = (abs_time - now).to_native();
            const i64 _ns =
                static_cast<i64>(_left.tv_sec) * 1'000'000'000 + _left.tv_nsec;

            if (_ns <= 0)
            {
                return 0;
            }

            const i64 _ms = (_ns + 999'999) / 1'000'000;
            if (_ms > static_cast<i64>(max_timeout_ms))
            {
                return max_timeout_ms;
            }

            return static_cast<u32>(_ms);
        }
    } // namespace cv_detail

    #if defined(HSD_PLATFORM_WINDOWS)
    /// @brief Windows condition-variable primitives wrapping the native
    /// slim reader/writer condition variable API.
    namespace cv_detail
    {
        /// @brief Native condition-variable handle type.
        using native_handle_type = CONDITION_VARIABLE;

        /// @brief Initializes a native condition variable in place.
        /// @param cv Handle to initialize.
        static inline void initialize(native_handle_type& cv)
        {
            InitializeConditionVariable(&cv);
        }

        /// @brief Atomically releases `lock` and blocks until woken.
        /// @param cv Condition variable to wait on.
        /// @param lock Mutex currently held by the caller; released while
        /// waiting and reacquired before returning.
        /// @details The mutex is an SRW lock held in exclusive mode, so the
        /// wait takes flags `0`.
        static inline void wait(
            native_handle_type& cv, mutex& lock)
        {
            SleepConditionVariableSRW(&cv, lock.native_handle(), INFINITE, 0);
        }

        /// @brief Atomically releases `lock` and blocks until woken or
        /// `abs_time` passes.
        /// @param cv Condition variable to wait on.
        /// @param lock Mutex currently held by the caller; released while
        /// waiting and reacquired before returning.
        /// @param abs_time Absolute deadline for the wait.
        static inline void wait_until(
            native_handle_type& cv, mutex& lock, const precise_clock& abs_time)
        {
            SleepConditionVariableSRW(
                &cv, lock.native_handle(),
                timeout_ms(abs_time, precise_clock::now()), 0
            );
        }
        
        /// @brief Wakes a single thread waiting on the condition variable.
        /// @param cv Condition variable to signal.
        static inline void notify_one(native_handle_type& cv)
        {
            WakeConditionVariable(&cv);
        }

        /// @brief Wakes every thread waiting on the condition variable.
        /// @param cv Condition variable to broadcast to.
        static inline void notify_all(native_handle_type& cv)
        {
            WakeAllConditionVariable(&cv);
        }

        /// @brief No-op: Windows condition variables need no explicit
        /// destruction.
        static inline void destroy(native_handle_type&)
        {
            // do nothing
        }
    } // namespace cv_detail
    #else
    /// @brief POSIX condition-variable primitives wrapping `pthread_cond_t`.
    namespace cv_detail
    {
        /// @brief Native condition-variable handle type.
        using native_handle_type = pthread_cond_t;

        /// @brief Initializes a native condition variable in place.
        /// @param cv Handle to initialize.
        static inline void initialize(native_handle_type& cv)
        {
            pthread_cond_init(&cv, nullptr);
        }

        /// @brief Atomically releases `lock` and blocks until woken.
        /// @param cv Condition variable to wait on.
        /// @param lock Mutex currently held by the caller; released while
        /// waiting and reacquired before returning.
        static inline void wait(
            native_handle_type& cv, mutex& lock)
        {
            pthread_cond_wait(&cv, lock.native_handle());
        }

        /// @brief Atomically releases `lock` and blocks until woken or
        /// `abs_time` passes.
        /// @param cv Condition variable to wait on.
        /// @param lock Mutex currently held by the caller; released while
        /// waiting and reacquired before returning.
        /// @param abs_time Absolute deadline for the wait.
        static inline void wait_until(
            native_handle_type& cv, mutex& lock, const precise_clock& abs_time)
        {
            auto _ts = abs_time.to_native();
            pthread_cond_timedwait(&cv, lock.native_handle(), &_ts);
        }

        /// @brief Wakes a single thread waiting on the condition variable.
        /// @param cv Condition variable to signal.
        static inline void notify_one(native_handle_type& cv)
        {
            pthread_cond_signal(&cv);
        }

        /// @brief Wakes every thread waiting on the condition variable.
        /// @param cv Condition variable to broadcast to.
        static inline void notify_all(native_handle_type& cv)
        {
            pthread_cond_broadcast(&cv);
        }

        /// @brief Destroys a native condition variable.
        /// @param cv Handle to destroy.
        static inline void destroy(native_handle_type& cv)
        {
            pthread_cond_destroy(&cv);
        }
    } // namespace cv_detail
    #endif

    /// @brief Result of a timed condition-variable wait.
    enum class cv_status
    {
        no_timeout, ///< The predicate/notification was observed before the deadline.
        timeout ///< The deadline elapsed before being woken.
    };

    /// @brief Predicate-based condition-variable synchronization primitive.
    /// @details Pairs with unique_lock<mutex> to atomically release the lock
    /// while waiting and reacquire it before returning, so a caller always
    /// holds the lock again when a wait call completes.
    class condition_variable
    {
    private:
        cv_detail::native_handle_type _cv; ///< Native condition-variable handle.

    public:
        /// @brief Constructs and initializes the native condition variable.
        inline condition_variable()
        {
            cv_detail::initialize(_cv);
        }

        /// @brief Initializes a fresh handle, then takes over `other`'s state.
        /// @param other Condition variable to move from.
        /// @details `other` is left holding a freshly initialized, valid
        /// handle (via the initial initialize() call) rather than a moved-
        /// from empty state, since the native handle types have no null
        /// state to swap into.
        inline condition_variable(condition_variable&& other)
        {
            cv_detail::initialize(_cv);
            hsd::swap(other._cv, _cv);
        }

        /// @brief Destroys the native condition variable.
        inline ~condition_variable()
        {
            cv_detail::destroy(_cv);
        }

        inline condition_variable(const condition_variable&) = delete;
        inline condition_variable& operator=(const condition_variable&) = delete;

        /// @brief Exchanges the native handle with `rhs`.
        /// @param rhs Condition variable to move from.
        /// @return Reference to this condition variable.
        inline condition_variable& operator=(condition_variable&& rhs)
        {
            hsd::swap(rhs._cv, _cv);
            return *this;
        }

        /// @brief Atomically unlocks `lock` and blocks until notified.
        /// @param lock Lock currently owning the associated mutex; released
        /// while waiting and reacquired before returning.
        /// @details Subject to spurious wakeups; callers needing a stable
        /// condition should use the predicate-based overload instead.
        inline void wait(unique_lock<mutex>& lock)
        {
            cv_detail::wait(_cv, *lock.get_mutex());
        }

        /// @brief Waits until `pred` returns true, re-checking after each
        /// wakeup.
        /// @tparam Pred Predicate type invocable with no arguments.
        /// @param lock Lock currently owning the associated mutex; released
        /// while waiting and reacquired before returning.
        /// @param pred Predicate polled under the lock after each wakeup;
        /// waiting stops once it returns true.
        template <typename Pred>
        inline void wait(unique_lock<mutex>& lock, Pred pred)
        {
            while (pred() == false)
            {
                wait(lock);
            }
        }

        /// @brief Atomically unlocks `lock` and blocks until notified or
        /// `abs_time` passes.
        /// @param lock Lock currently owning the associated mutex; released
        /// while waiting and reacquired before returning.
        /// @param abs_time Absolute deadline for the wait.
        /// @return `cv_status::timeout` if `abs_time` has already passed once
        /// the wait returns, otherwise `cv_status::no_timeout`.
        inline cv_status wait_until(
            unique_lock<mutex>& lock, const precise_clock& abs_time)
        {
            cv_detail::wait_until(_cv, *lock.get_mutex(), abs_time);

            if (abs_time < precise_clock::now())
            {
                return cv_status::timeout;
            }

            return cv_status::no_timeout;
        }

        /// @brief Waits until `pred` returns true or `abs_time` passes.
        /// @tparam Pred Predicate type invocable with no arguments.
        /// @param lock Lock currently owning the associated mutex; released
        /// while waiting and reacquired before returning.
        /// @param abs_time Absolute deadline for the wait.
        /// @param pred Predicate polled under the lock after each wakeup.
        /// @return The final result of `pred()`: true if satisfied before the
        /// deadline, false if the deadline elapsed with the predicate still
        /// false.
        template <typename Pred>
        inline bool wait_until(
            unique_lock<mutex>& lock, const precise_clock& abs_time, Pred pred)
        {
            while (pred() == false)
            {
                if (wait_until(lock, abs_time) == cv_status::timeout)
                    return pred();
            }

            return true;
        }

        /// @brief Atomically unlocks `lock` and blocks until notified or
        /// `rel_time` elapses.
        /// @param lock Lock currently owning the associated mutex; released
        /// while waiting and reacquired before returning.
        /// @param rel_time Duration to wait, measured from now.
        /// @return `cv_status::timeout` if `rel_time` elapsed before being
        /// woken, otherwise `cv_status::no_timeout`.
        inline cv_status wait_for(
            unique_lock<mutex>& lock, const precise_clock& rel_time)
        {
            return wait_until(lock, precise_clock::now() + rel_time);
        }

        /// @brief Waits until `pred` returns true or `rel_time` elapses.
        /// @tparam Pred Predicate type invocable with no arguments.
        /// @param lock Lock currently owning the associated mutex; released
        /// while waiting and reacquired before returning.
        /// @param rel_time Duration to wait, measured from now.
        /// @param pred Predicate polled under the lock after each wakeup.
        /// @return The final result of `pred()`: true if satisfied before the
        /// deadline, false if the deadline elapsed with the predicate still
        /// false.
        template <typename Pred>
        inline bool wait_for(
            unique_lock<mutex>& lock, const precise_clock& rel_time, Pred pred)
        {
            return wait_until(lock, precise_clock::now() + rel_time, hsd::move(pred));
        }

        /// @brief Wakes a single thread waiting on this condition variable.
        inline void notify_one()
        {
            cv_detail::notify_one(_cv);
        }

        /// @brief Wakes every thread waiting on this condition variable.
        inline void notify_all()
        {
            cv_detail::notify_all(_cv);
        }
    };
} // namespace hsd
