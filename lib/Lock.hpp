#pragma once

#include "Atomic.hpp"
#include "Result.hpp"

#if defined(HSD_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(HSD_PLATFORM_POSIX)
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

namespace hsd
{
    #if defined(HSD_PLATFORM_WINDOWS)
    namespace futex_detail
    {
        static inline bool wait(void* value, u32 expected)
        {
            return WaitOnAddress(value, &expected, sizeof(expected), -1);
        }

        static inline void wake(void* value)
        {
            WakeByAddressAll(value);
        }
    } // namespace futex_detail

    namespace mutex_detail
    {
        using handle_type = CRITICAL_SECTION;

        static inline handle_type create()
        {
            handle_type _handle;
            InitializeCriticalSection(&_handle);
            return _handle;
        }

        static inline void destroy(handle_type& mutex)
        {
            DeleteCriticalSection(&mutex);
        }

        static inline void lock(handle_type& mutex)
        {
            EnterCriticalSection(&mutex);
        }

        static inline bool try_lock(handle_type& mutex)
        {
            return TryEnterCriticalSection(&mutex);
        }

        static inline void unlock(handle_type& mutex)
        {
            LeaveCriticalSection(&mutex);
        }
    } // namespace mutex_detail
    #elif defined(HSD_PLATFORM_POSIX)
    #if defined(SYS_futex)
    namespace futex_detail
    {
        static inline bool wait(void* value, u32 expected)
        {
            return syscall(
                SYS_futex, reinterpret_cast<u32*>(value), 
                0, expected, nullptr, nullptr, 0
            ) == 0;
        }

        static inline void wake(void* value)
        {
            syscall(
                SYS_futex, reinterpret_cast<u32*>(value), 
                1, 2147483647, nullptr, nullptr, 0
            );
        }
    } // namespace futex_detail
    #endif

    namespace mutex_detail
    {
        using handle_type = pthread_mutex_t;

        static inline handle_type create()
        {
            handle_type _mutex;
            pthread_mutex_init(&_mutex, nullptr);
            return _mutex;
        }

        static inline void destroy(handle_type& mutex)
        {
            pthread_mutex_destroy(&mutex);
        }

        static inline void lock(handle_type& mutex)
        {
            pthread_mutex_lock(&mutex);
        }

        static inline bool try_lock(handle_type& mutex)
        {
            return pthread_mutex_trylock(&mutex) == 0;
        }

        static inline void unlock(handle_type& mutex)
        {
            pthread_mutex_unlock(&mutex);
        }
    } // namespace mutex_detail
    #endif

    #if defined(HSD_PLATFORM_WINDOWS) || (defined(HSD_PLATFORM_POSIX) && defined(SYS_futex))
    struct futex_lock
    {
        inline bool wait_on(u32& addr, u32 cmp_addr)
        {
            return futex_detail::wait(&addr, cmp_addr);
        }

        inline void wake_up(u32& addr)
        {
            futex_detail::wake(&addr);
        }
    };
    #else
    class futex_lock
    {
    private:
        atomic_u32 _lock;

    public:
        inline futex_lock()
            : _lock{0}
        {}

        inline futex_lock(const futex_lock&) = delete;
        inline futex_lock &operator=(const futex_lock&) = delete;

        inline futex_lock(futex_lock&& other)
            : _lock{other._lock.load()}
        {
            other._lock = 0;
        }

        inline futex_lock& operator=(futex_lock&& rhs)
        {
            rhs._lock = _lock.exchange(
                rhs._lock.load(), memory_order_acquire
            );
            return *this;
        }

        inline bool wait_on(u32& addr, u32 cmp_addr)
        {
            if (_lock.load() == 0)
            {
                _lock.store(addr);
            }
            if (addr != cmp_addr)
            {
                return true;
            }
            else
            {
                while (_lock.load(memory_order_acquire))
                    ;

                return true;
            }
        }

        inline void wake_up(u32& addr)
        {
            if (_lock.load() != addr)
            {
                _lock.store(0, memory_order_release);
            }
        }
    };
    #endif

    class futex
    {
    private:
        atomic_i32 _lock;
        static inline futex_lock waiter{};
        static constexpr inline u32 waitersBit = (1 << 31);

    public:
        inline futex()
            : _lock{0}
        {}

        inline futex(const futex&) = delete;
        inline futex& operator=(const futex&) = delete;

        inline futex(futex&& other)
            : _lock{other._lock.load()}
        {
            other._lock = 0;
        }

        inline futex& operator=(futex&& rhs)
        {
            rhs._lock = _lock.exchange(
                rhs._lock.load(), memory_order_acquire
            );

            return *this;
        }

        inline void lock()
        {
            u32 expected = 0;

            while (true)
            {
                if (!expected)
                {
                    // Try to take the mutex here.
                    if (_lock.compare_exchange_strong(
                            reinterpret_cast<i32&>(expected), 1,
                            memory_order_acquire, memory_order_acquire
                        )
                    )
                    {
                        return;
                    }
                }
                else
                {
                    if (expected & waitersBit)
                    {
                        if (!waiter.wait_on(reinterpret_cast<u32&>(_lock), expected))
                            hsd_panic("waiter.wait_on() failed");

                        expected = 0;
                    }
                    else
                    {
                        u32 desired = expected | waitersBit;
                        if (_lock.compare_exchange_strong(
                                reinterpret_cast<i32&>(expected), desired,
                                memory_order_relaxed, memory_order_relaxed
                            )
                        )
                        {
                            expected = desired;
                        }
                    }
                }
            }
        }

        inline void unlock()
        {
            if (_lock.exchange(0, memory_order_release) & waitersBit)
                waiter.wake_up(reinterpret_cast<u32&>(_lock));
        }

        inline auto* native_handle()
        {
            return &_lock;
        }
    };

    class mutex
    {
    private:
        mutex_detail::handle_type _mutex;

    public:
        inline mutex()
        {
            _mutex = mutex_detail::create();
        }

        inline ~mutex()
        {
            mutex_detail::destroy(_mutex);
        }

        inline mutex(const mutex&) = delete;
        inline mutex& operator=(const mutex&) = delete;

        inline mutex(mutex&& other)
            : _mutex{other._mutex}
        {
            other._mutex = mutex_detail::handle_type{};
        }

        inline mutex& operator=(mutex&& rhs)
        {
            swap(_mutex, rhs._mutex);
            return *this;
        }

        inline void lock()
        {
            mutex_detail::lock(_mutex);
        }

        inline bool try_lock()
        {
            return mutex_detail::try_lock(_mutex);
        }

        inline void unlock()
        {
            mutex_detail::unlock(_mutex);
        }

        inline auto* native_handle()
        {
            return &_mutex;
        }
    };

    class spin
    {
    private:
        atomic_flag _spin{};

    public:
        inline spin() = default;

        inline void lock()
        {
            while (_spin.test_and_set(memory_order_acquire))
                ;
        }

        inline bool try_lock()
        {
            return !_spin.test_and_set(memory_order_acquire);
        }

        inline void unlock()
        {
            _spin.clear(memory_order_release);
        }

        inline auto* native_handle()
        {
            return &_spin;
        }
    };

    struct dont_lock_t {};
    struct adopt_lock_t {};

    static constexpr auto dont_lock = dont_lock_t{};
    static constexpr auto adopt_lock = adopt_lock_t{};

    template <typename Mutex>
    class unique_lock
    {
    private:
        Mutex* _mutex;
        bool _is_locked;

    public:
        inline friend void swap(unique_lock& lhs, unique_lock& rhs)
        {
            hsd::swap(lhs._mutex, rhs._mutex);
            hsd::swap(lhs._is_locked, rhs._is_locked);
        }

        inline unique_lock()
            : _mutex{nullptr}, _is_locked{false} {}

        inline unique_lock(dont_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false} {}

        inline unique_lock(adopt_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{true} {}

        inline unique_lock(Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false}
        {
            lock().unwrap();
        }

        inline unique_lock(const unique_lock&) = delete;
        inline unique_lock& operator=(const unique_lock&) = delete;

        inline unique_lock(unique_lock&& other)
            : unique_lock{}
        {
            swap(*this, other);
        }

        inline ~unique_lock()
        {
            if (_is_locked == true)
                unlock().unwrap();
        }

        inline unique_lock& operator=(unique_lock&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline Result<void, runtime_error> lock()
        {
            if (_is_locked == true)
            {
                return runtime_error {
                    "unique_lock::lock(): mutex already locked"
                };
            }

            _mutex->lock();
            _is_locked = true;
            return {};
        }

        inline Result<void, runtime_error> unlock()
        {
            if (_is_locked == false)
            {
                return runtime_error {
                    "unique_lock::unlock(): mutex not locked"
                };
            }
            
            _mutex->unlock();
            _is_locked = false;
            return {};
        }
        
        inline Mutex* get_mutex() const
        {
            return _mutex;
        }

        inline bool is_locked()
        {
            return _is_locked;
        }

        inline bool protects(Mutex* mutex)
        {
            return _is_locked && mutex == _mutex;
        }
    };

    template <typename Mutex>
    class shared_lock
    {
    private:
        Mutex* _mutex;
        bool _is_locked;

    public:
        inline friend void swap(shared_lock& lhs, shared_lock& rhs)
        {
            hsd::swap(lhs._mutex, rhs._mutex);
            hsd::swap(lhs._is_locked, rhs._is_locked);
        }

        inline shared_lock()
            : _mutex{nullptr}, _is_locked{false}
        {}

        inline shared_lock(dont_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false}
        {}

        inline shared_lock(adopt_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{true}
        {}

        inline shared_lock(Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false}
        {
            lock().unwrap();
        }

        inline shared_lock(const shared_lock&) = delete;
        inline shared_lock& operator=(const shared_lock&) = delete;

        inline shared_lock(shared_lock&& other)
            : shared_lock{}
        {
            swap(*this, other);
        }

        inline ~shared_lock()
        {
            if (_is_locked == true)
                unlock().unwrap();
        }

        inline shared_lock& operator=(shared_lock&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline Result<void, runtime_error> lock()
        {
            if (_is_locked == true)
            {
                return runtime_error {
                    "shared_lock::lock(): mutex already locked"
                };
            }

            _mutex->lock_shared();
            _is_locked = true;
            return {};
        }

        inline Result<void, runtime_error> unlock()
        {
            if (_is_locked == false)
            {
                return runtime_error {
                    "shared_lock::unlock(): mutex not locked"
                };
            }

            _mutex->unlock_shared();
            _is_locked = false;
            return {};
        }

        inline Mutex* get_mutex() const
        {
            return &_mutex;
        }

        inline bool is_locked()
        {
            return _is_locked;
        }

        inline bool protects(Mutex* mutex)
        {
            return _is_locked && mutex == _mutex;
        }
    };

    template <typename Mutex>
    unique_lock<Mutex> guard(Mutex* mutex)
    {
        return unique_lock<Mutex>(*mutex);
    }

    template <typename Mutex>
    unique_lock<Mutex> guard(dont_lock_t, Mutex* mutex)
    {
        return unique_lock<Mutex>(dont_lock, *mutex);
    }
} // namespace hsd
