#pragma once

/// @file Thread.hpp
/// @brief Thread creation, joining, detaching, and callable binding.
/// @details Decays callable arguments into owned thread state and exposes
/// portable identifiers, sleeping, yielding, and lifecycle operations.

#include "../Wrappers/Tuple.hpp"
#include "../Base/Result.hpp"

#include <stdlib.h>

#if defined(HSD_PLATFORM_WINDOWS)
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#endif

namespace hsd 
{
    #if defined(HSD_PLATFORM_POSIX)
    /// @brief Native thread identifier type: POSIX handle widened to u64.
    using native_id_type = u64;
    #else
    /// @brief Native thread identifier type: Windows thread id.
    using native_id_type = DWORD;
    #endif
    
    /// @brief Portable comparable thread identifier.
    struct id
    {	
        /// @brief Constructs a null identifier, equal to no running thread.
        id() = default;

        /// @brief Wraps a native thread identifier.
        /// @param id Platform-native thread id value.
        id(native_id_type id) 
            : _id{id}
        {}

        /// @brief Compares two identifiers for the same underlying thread.
        /// @param lhs First identifier.
        /// @param rhs Second identifier.
        /// @return True if both wrap the same native id.
        friend bool operator==(id lhs, id rhs) 
        {
            return lhs._id == rhs._id;
        }

    private:
        native_id_type _id; ///< Native thread identifier value.
    };

    /// @brief Operations applying to the calling thread.
    struct this_thread
    {
        /// @brief Gets the identifier of the calling thread.
        /// @return Portable id wrapping the native thread identifier.
        static inline id get_id() 
        {
            #if defined(HSD_PLATFORM_POSIX)
            return id {
                static_cast<native_id_type>(pthread_self())
            };
            #else
            return id {
                static_cast<native_id_type>(GetCurrentThreadId())
            };
            #endif
        }

        /// @brief Blocks the calling thread for the given duration.
        /// @param seconds Duration to sleep for; may include a fractional
        /// part (converted to nanosecond/millisecond resolution).
        /// @return Empty on success, or the errno-derived error if a
        /// non-interrupt failure occurs. On POSIX, an interrupted sleep
        /// (EINTR) is retried automatically with the remaining time.
        static inline option_err<runtime_error> sleep_for(f32 seconds)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
                Sleep(seconds * 1000); // milliseconds
            #else /* posix */
            auto frac = seconds - static_cast<i32>(seconds);
            timespec ts {
                .tv_sec = static_cast<i32>(seconds),
                .tv_nsec = static_cast<long>(frac * 1'000'000'000.f)
            };

            while (nanosleep(&ts, &ts) == -1)
            {
                if (errno != EINTR)
                    return runtime_error{strerror(errno)};
            }
            #endif

            return {};
        }

        /// @brief Yields the remainder of the calling thread's time slice.
        static inline void yield()
        {
            #if defined(HSD_PLATFORM_WINDOWS)
                Sleep(0);
            #else
                sched_yield();
            #endif
        }
    };

    /// @brief Move-only thread owner with join and detach operations.
    struct thread 
    {
        #if defined(HSD_PLATFORM_POSIX)
        /// @brief Native platform thread handle type.
        using native_handle_type = pthread_t;
        #else
        /// @brief Native platform thread handle type.
        using native_handle_type = HANDLE;
        #endif

        /// @brief Constructs a non-joinable thread object.
        inline thread() = default;

        /// @brief Aborts the process if the thread is still joinable.
        /// @details A live, joinable thread must be explicitly joined or
        /// detached before destruction, matching std::thread semantics;
        /// this is enforced here by aborting rather than throwing, since the
        /// library builds without exceptions.
        inline ~thread() 
        {
            if (joinable())
                abort();
        }

        /// @brief Starts a new thread running the given callable with
        /// arguments.
        /// @tparam Func Callable type invocable with the decayed arguments.
        /// @tparam Args Argument types bound and forwarded to the callable.
        /// @param func Callable to run on the new thread.
        /// @param args Arguments to bind and pass to the callable.
        /// @details Decay-copies `func` and `args` into a heap-allocated
        /// block whose ownership transfers to the new thread; the thread's
        /// entry point applies the callable to the stored arguments and
        /// deletes the block itself before returning, so the caller performs
        /// no explicit cleanup. Aborts the process if the underlying native
        /// thread cannot be created.
        template <typename Func, typename... Args>
        requires (InvocableRet<void, Func, Args...>)
        inline thread(Func&& func, Args&&... args) 
        {
            #if defined(HSD_PLATFORM_POSIX)
            pthread_attr_t _attr;
            pthread_attr_init(&_attr);
            #endif

            // Copies (never references) the callable/argument state so it
            // outlives the calling scope for the lifetime of the new thread.
            auto decay_copy = []<typename T>(T&& t) -> decay_t<T>
            {
                return hsd::forward<T>(t);
            };

            using func_type = decay_t<Func>;
            using args_type = tuple<decay_t<Args>...>;
            
            #if defined(HSD_PLATFORM_WINDOWS)
            using return_type = DWORD;
            #else	
            using return_type = void*;
            #endif

            // Heap-allocated ownership handoff to the new thread: built here
            // on the calling thread, freed by enter_thread on the new thread.
            struct thread_data 
            {
                func_type func;
                args_type args;

                // Thread entry point: unpacks the stored callable/arguments,
                // invokes them, then deletes this block before returning.
                static return_type enter_thread(void* arg)
                {
                    auto* td = reinterpret_cast<thread_data*>(arg);
                    apply(td->func, td->args);

                    delete td;
                    return static_cast<return_type>(0);
                }
            }; 
            
            auto* _td = new thread_data
            {
                .func = decay_copy(hsd::forward<Func>(func)),
                .args = make_tuple(decay_copy(hsd::forward<Args>(args))...)
            };

            #if defined(HSD_PLATFORM_WINDOWS)
            native_id_type _native_id;
            _handle = CreateThread(
                nullptr, 0, thread_data::enter_thread, _td, 0, &_native_id
            );
            
            if (!_handle)
            {
                abort();
            }
            
            _id = id{static_cast<native_id_type>(_native_id)};
            #else
            i32 _res = pthread_create(&_handle, &_attr, thread_data::enter_thread, _td);
            
            if (_res != 0)
            {
                abort();
            }
            
            _id = id{static_cast<native_id_type>(_handle)};
            #endif
        }

        inline thread(const thread&) = delete;
        inline thread& operator=(const thread&) = delete;

        /// @brief Transfers ownership of another thread's handle and id.
        /// @param other Thread to move from; left non-joinable.
        inline thread(thread&& other) 
        {
            swap(other); 
        }

        /// @brief Replaces this thread's state by exchanging with `other`.
        /// @param other Thread to move from.
        /// @return Reference to this thread.
        inline thread& operator=(thread&& other) 
        {
            swap(other);
            return *this;
        }

        /// @brief Exchanges the native handle and id with another thread.
        /// @param other Thread to swap state with.
        inline void swap(thread& other)
        { 
            // namespace needed
            hsd::swap(_handle, other._handle);
            hsd::swap(_id, other._id);
        }

        /// @brief Gets the identifier of this thread object.
        /// @return Portable id, or a null id if not joinable.
        inline id get_id() const
        {
            return id{_id};
        }

        /// @brief Checks whether the thread refers to an active execution.
        /// @return True if the thread has not yet been joined or detached.
        inline bool joinable() 
        {
            return _id != id{};
        }

        /// @brief Gets the platform-native thread handle.
        /// @return Native handle value; meaningless once join()/detach() has
        /// run.
        inline native_handle_type native_handle() 
        {
            return _handle;
        }

        /// @brief Queries the number of concurrent hardware threads
        /// supported by the platform.
        /// @return Implementation-defined hardware thread count.
        static inline auto hardware_concurrency() 
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            SYSTEM_INFO info;
            GetSystemInfo(&info);
            return info.dwNumberOfProcessors;
            #else
            return sysconf(_SC_NPROCESSORS_ONLN);
            #endif
        }

        /// @brief Detaches the thread, letting it run independently.
        /// @return Empty on success, or an error if the thread is not
        /// joinable.
        /// @details After a successful detach, the thread is no longer
        /// joinable and its native resources are released automatically
        /// once it finishes running.
        inline option_err<runtime_error> detach() 
        {
            if (!joinable())
            {
                return runtime_error{"Current Thread cannot be detached"};
            }

            #if defined(HSD_PLATFORM_POSIX)
            pthread_detach(_handle);
            _handle = 0;
            #else
            CloseHandle(_handle);
            _handle = nullptr;
            #endif
            
            _id = {};
            return {};
        }

        /// @brief Blocks the caller until this thread finishes execution.
        /// @return Empty on success, or an error if the thread is not
        /// joinable.
        inline option_err<runtime_error> join() 
        {
            if (!joinable())
            {
                return runtime_error{"Current Thread cannot be joined"};
            }

            #if defined(HSD_PLATFORM_POSIX)
            pthread_join(_handle, nullptr);
            _handle = 0;
            #else
            WaitForSingleObject(_handle, INFINITE);
            CloseHandle(_handle);
            _handle = nullptr;
            #endif

            _id = {};
            return {};
        }
    
    private:
        native_handle_type _handle{}; ///< Native platform thread handle.
        id _id{}; ///< Cached identifier; null when not joinable.
    }; // struct thread
} // namespace hsd