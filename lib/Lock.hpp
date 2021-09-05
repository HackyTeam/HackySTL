#pragma once

#include "Atomic.hpp"
#include "Concepts.hpp"
#include <assert.h>

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
    #elif defined(HSD_PLATFORM_POSIX)
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
                1, 1, nullptr, nullptr, 0
            );
        }
    } // namespace futex_detail
    #endif

    class spin_lock
    {
    private:
        atomic_u32 _lock;

    public:
        inline spin_lock()
            : _lock{0}
        {}

        inline spin_lock(const spin_lock&) = delete;
        inline spin_lock &operator=(const spin_lock&) = delete;

        inline spin_lock(spin_lock&& other)
            : _lock{other._lock.load()}
        {
            other._lock = 0;
        }

        inline spin_lock& operator=(spin_lock&& rhs)
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

    template <DefaultConstructible Waiter>
    class Locker
    {
    private:
        atomic_i32 _lock;
        static inline Waiter waiter{};
        static constexpr inline u32 waitersBit = (1 << 31);

    public:
        inline Locker()
            : _lock{0}
        {}

        inline Locker(const Locker&) = delete;
        inline Locker& operator=(const Locker&) = delete;

        inline Locker(Locker&& other)
            : _lock{other._lock.load()}
        {
            other._lock = 0;
        }

        inline Locker& operator=(Locker&& rhs)
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
                            assert(!"sys_futex_wait() failed");

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
    };

    using mutex = Locker<spin_lock>;
    using futex = Locker<futex_lock>;

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
            lock();
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
            if (_is_locked)
                unlock();
        }

        inline unique_lock& operator=(unique_lock&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline void lock()
        {
            assert(!_is_locked);
            _mutex->lock();
            _is_locked = true;
        }

        inline void unlock()
        {
            assert(_is_locked);
            _mutex->unlock();
            _is_locked = false;
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
            lock();
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
            if (_is_locked)
                unlock();
        }

        inline shared_lock& operator=(shared_lock&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline void lock()
        {
            assert(!_is_locked);
            _mutex->lock_shared();
            _is_locked = true;
        }

        inline void unlock()
        {
            assert(_is_locked);
            _mutex->unlock_shared();
            _is_locked = false;
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
