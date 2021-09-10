#pragma once

#include "Lock.hpp"
#include "Time.hpp"

namespace hsd
{
    #if defined(HSD_PLATFORM_WINDOWS)
    namespace cv_detail
    {
        using native_handle_type = CONDITION_VARIABLE;

        static inline void initialize(native_handle_type& cv)
        {
            InitializeConditionVariable(&cv);
        }

        static inline void wait(
            native_handle_type& cv, mutex& lock)
        {
            SleepConditionVariableCS(&cv, lock.native_handle(), -1);
        }

        static inline void wait_until(
            native_handle_type& cv, mutex& lock, const precise_clock& abs_time)
        {
            SleepConditionVariableCS(
                &cv, lock.native_handle(), abs_time.to_miliseconds()
            );
        }
        
        static inline void notify_one(native_handle_type& cv)
        {
            WakeConditionVariable(&cv);
        }

        static inline void notify_all(native_handle_type& cv)
        {
            WakeAllConditionVariable(&cv);
        }

        static inline void destroy(native_handle_type&)
        {
            // do nothing
        }
    } // namespace cv_detail
    #else
    namespace cv_detail
    {
        using native_handle_type = pthread_cond_t;

        static inline void initialize(native_handle_type& cv)
        {
            pthread_cond_init(&cv, nullptr);
        }

        static inline void wait(
            native_handle_type& cv, mutex& lock)
        {
            pthread_cond_wait(&cv, lock.native_handle());
        }

        static inline void wait_until(
            native_handle_type& cv, mutex& lock, const precise_clock& abs_time)
        {
            auto _ts = abs_time.to_native();
            pthread_cond_timedwait(&cv, lock.native_handle(), &_ts);
        }

        static inline void notify_one(native_handle_type& cv)
        {
            pthread_cond_signal(&cv);
        }

        static inline void notify_all(native_handle_type& cv)
        {
            pthread_cond_broadcast(&cv);
        }

        static inline void destroy(native_handle_type& cv)
        {
            pthread_cond_destroy(&cv);
        }
    } // namespace cv_detail
    #endif

    enum class cv_status
    {
        no_timeout,
        timeout
    };

    class condition_variable
    {
    private:
        cv_detail::native_handle_type _cv;

    public:
        inline condition_variable()
        {
            cv_detail::initialize(_cv);
        }

        inline condition_variable(condition_variable&& other)
        {
            cv_detail::initialize(_cv);
            swap(other._cv, _cv);
        }

        inline ~condition_variable()
        {
            cv_detail::destroy(_cv);
        }

        inline condition_variable(const condition_variable&) = delete;
        inline condition_variable& operator=(const condition_variable&) = delete;

        inline condition_variable& operator=(condition_variable&& rhs)
        {
            swap(rhs._cv, _cv);
            return *this;
        }

        inline void wait(unique_lock<mutex>& lock)
        {
            cv_detail::wait(_cv, *lock.get_mutex());
        }

        template <typename Pred>
        inline void wait(unique_lock<mutex>& lock, Pred pred)
        {
            while (pred() == false)
            {
                wait(lock);
            }
        }

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

        inline cv_status wait_for(
            unique_lock<mutex>& lock, const precise_clock& rel_time)
        {
            return wait_until(lock, precise_clock::now() + rel_time);
        }

        template <typename Pred>
        inline bool wait_for(
            unique_lock<mutex>& lock, const precise_clock& rel_time, Pred pred)
        {
            return wait_until(lock, precise_clock::now() + rel_time, move(pred));
        }

        inline void notify_one()
        {
            cv_detail::notify_one(_cv);
        }

        inline void notify_all()
        {
            cv_detail::notify_all(_cv);
        }
    };
} // namespace hsd
