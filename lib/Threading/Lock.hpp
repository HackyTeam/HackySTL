#pragma once

/// @file Lock.hpp
/// @brief Mutex, futex, spin-lock, and unique-lock abstractions.
/// @details Provides scoped ownership and explicit lock/unlock operations for
/// platform threading primitives with result-based failure reporting.

#include "Atomic.hpp"
#include "../Base/Result.hpp"

#include <limits.h>

#if defined(HSD_PLATFORM_WINDOWS)
#define _WINSOCKAPI_
#include <windows.h>
#elif defined(HSD_PLATFORM_POSIX)
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

namespace hsd
{
    #if defined(HSD_PLATFORM_WINDOWS)
    namespace futex_detail
    {
        /// @brief Waits while `value` equals `expected` using the platform futex.
        /// @param value Address containing the observed word.
        /// @param expected Value that permits blocking.
        /// @return Platform wait success.
        static inline bool wait(void* value, u32 expected)
        {
            return WaitOnAddress(value, &expected, sizeof(expected), INFINITE);
        }

        /// @brief Wakes all platform waiters associated with an address.
        /// @param value Address whose waiters are woken.
        static inline void wake(void* value)
        {
            WakeByAddressAll(value);
        }
    } // namespace futex_detail

    namespace mutex_detail
    {
        /// @brief Native mutex handle type selected for the platform.
        /// @details A slim reader/writer lock used in exclusive mode. A critical
        /// section is recursive, so `try_lock` would succeed for the thread that
        /// already owns it, which a non-recursive mutex must not do.
        using handle_type = SRWLOCK;

        /// @brief Creates and initializes a native mutex handle.
        /// @return Initialized native handle.
        static inline handle_type create()
        {
            return SRWLOCK_INIT;
        }

        /// @brief Destroys a native mutex handle. An SRW lock needs no cleanup.
        static inline void destroy(handle_type&)
        {}

        /// @brief Blocks until a native mutex is acquired.
        /// @param mutex Handle to lock.
        static inline void lock(handle_type& mutex)
        {
            AcquireSRWLockExclusive(&mutex);
        }

        /// @brief Attempts to acquire a native mutex without blocking.
        /// @param mutex Handle to lock.
        /// @return True when the mutex was acquired.
        static inline bool try_lock(handle_type& mutex)
        {
            return TryAcquireSRWLockExclusive(&mutex) != 0;
        }

        /// @brief Releases a native mutex.
        /// @param mutex Handle to unlock.
        static inline void unlock(handle_type& mutex)
        {
            ReleaseSRWLockExclusive(&mutex);
        }
    } // namespace mutex_detail
    #elif defined(HSD_PLATFORM_POSIX)
    #if defined(SYS_futex)
    namespace futex_detail
    {
        /// @brief Waits while `value` equals `expected` using Linux futex.
        /// @details `EAGAIN` means the word had already changed before the thread
        /// slept, and `EINTR` means a signal interrupted the wait. Both are normal
        /// wakeups: the caller re-checks the lock after every return.
        /// @return False only for a real failure of the syscall.
        static inline bool wait(void* value, u32 expected)
        {
            if (syscall(
                    SYS_futex, reinterpret_cast<u32*>(value),
                    0, expected, nullptr, nullptr, 0
                ) == 0)
            {
                return true;
            }

            return errno == EAGAIN || errno == EINTR;
        }

        /// @brief Wakes all Linux futex waiters associated with an address.
        /// @param value Address whose waiters are woken.
        static inline void wake(void* value)
        {
            syscall(
                SYS_futex, reinterpret_cast<u32*>(value), 
                1, INT_MAX, nullptr, nullptr, 0
            );
        }
    } // namespace futex_detail
    #endif

    namespace mutex_detail
    {
        /// @brief Native pthread mutex handle type.
        using handle_type = pthread_mutex_t;

        /// @brief Creates and initializes a pthread mutex.
        /// @return Initialized native handle.
        static inline handle_type create()
        {
            handle_type _mutex;
            pthread_mutex_init(&_mutex, nullptr);
            return _mutex;
        }

        /// @brief Destroys a pthread mutex.
        /// @param mutex Handle to destroy.
        static inline void destroy(handle_type& mutex)
        {
            pthread_mutex_destroy(&mutex);
        }

        /// @brief Blocks until a pthread mutex is acquired.
        /// @param mutex Handle to lock.
        static inline void lock(handle_type& mutex)
        {
            pthread_mutex_lock(&mutex);
        }

        /// @brief Attempts to acquire a pthread mutex without blocking.
        /// @param mutex Handle to lock.
        /// @return True when the mutex was acquired.
        static inline bool try_lock(handle_type& mutex)
        {
            return pthread_mutex_trylock(&mutex) == 0;
        }

        /// @brief Releases a pthread mutex.
        /// @param mutex Handle to unlock.
        static inline void unlock(handle_type& mutex)
        {
            pthread_mutex_unlock(&mutex);
        }
    } // namespace mutex_detail
    #endif

    #if defined(HSD_PLATFORM_WINDOWS) || (defined(HSD_PLATFORM_POSIX) && defined(SYS_futex))
    /// @brief Platform futex wait/wake adapter.
    struct futex_lock
    {
        /// @brief Waits while an address contains a comparison value.
        /// @param addr Address observed by the platform wait.
        /// @param cmp_addr Value that permits blocking.
        /// @return Platform wait result.
        inline bool wait_on(u32& addr, u32 cmp_addr)
        {
            return futex_detail::wait(&addr, cmp_addr);
        }

        /// @brief Wakes waiters associated with an address.
        /// @param addr Address whose waiters are woken.
        inline void wake_up(u32& addr)
        {
            futex_detail::wake(&addr);
        }
    };
    #else
    /// @brief Atomic fallback wait/wake adapter when native futexes are unavailable.
    class futex_lock
    {
    private:
        /// @brief Internal waiter state used by the fallback spin/wait protocol.
        atomic_u32 _lock;

    public:
        /// @brief Constructs an unlocked fallback waiter.
        inline futex_lock()
            : _lock{0}
        {}

        inline futex_lock(const futex_lock&) = delete;
        inline futex_lock &operator=(const futex_lock&) = delete;

        /// @brief Transfers fallback waiter state from `other`.
        /// @param other Waiter whose state is moved.
        inline futex_lock(futex_lock&& other)
            : _lock{other._lock.load()}
        {
            other._lock = 0;
        }

        /// @brief Transfers fallback waiter state from `rhs`.
        /// @param rhs Waiter whose state is moved.
        /// @return `*this` after assignment.
        inline futex_lock& operator=(futex_lock&& rhs)
        {
            rhs._lock = _lock.exchange(
                rhs._lock.load(), memory_order_acquire
            );
            return *this;
        }

        /// @brief Waits until the observed address changes from `cmp_addr`.
        /// @param addr Address observed by the fallback protocol.
        /// @param cmp_addr Value that permits blocking.
        /// @return True after the address is observed or awakened.
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

        /// @brief Releases the fallback waiter when its address no longer matches.
        /// @param addr Address observed by the fallback protocol.
        inline void wake_up(u32& addr)
        {
            if (_lock.load() != addr)
            {
                _lock.store(0, memory_order_release);
            }
        }
    };
    #endif

    /// @brief Blocking futex-style mutual exclusion primitive.
    class futex
    {
    private:
        /// @brief Atomic state containing ownership and waiter bits.
        atomic_i32 _lock;
        /// @brief Shared platform wait/wake adapter.
        static inline futex_lock waiter{};
        /// @brief High bit marking that waiters may need waking.
        static constexpr inline u32 waitersBit = (1u << 31);

    public:
        /// @brief Constructs an unlocked futex.
        inline futex()
            : _lock{0}
        {}

        inline futex(const futex&) = delete;
        inline futex& operator=(const futex&) = delete;

        /// @brief Transfers futex state from `other`.
        /// @param other Futex whose state is moved.
        inline futex(futex&& other)
            : _lock{other._lock.load()}
        {
            other._lock = 0;
        }

        /// @brief Transfers futex state from `rhs`.
        /// @param rhs Futex whose state is moved.
        /// @return `*this` after assignment.
        inline futex& operator=(futex&& rhs)
        {
            rhs._lock = _lock.exchange(
                rhs._lock.load(), memory_order_acquire
            );

            return *this;
        }

        /// @brief Blocks until this futex is acquired.
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
                            hsd::panic("waiter.wait_on() failed");

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

        /// @brief Releases this futex and wakes a waiter when marked.
        inline void unlock()
        {
            if (_lock.exchange(0, memory_order_release) & waitersBit)
                waiter.wake_up(reinterpret_cast<u32&>(_lock));
        }

        /// @brief Returns the atomic native state address.
        /// @return Pointer to the underlying lock state.
        inline auto* native_handle()
        {
            return &_lock;
        }
    };

    /// @brief Platform mutex wrapper with checked locking operations.
    class mutex
    {
    private:
        /// @brief Platform-native mutex handle owned by this wrapper.
        mutex_detail::handle_type _mutex;

    public:
        /// @brief Creates an unlocked platform mutex.
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

        /// @brief Transfers native mutex ownership from `other`.
        /// @param other Mutex whose handle is moved.
        inline mutex(mutex&& other)
            : _mutex{other._mutex}
        {
            other._mutex = mutex_detail::handle_type{};
        }

        /// @brief Exchanges native handles with `rhs`.
        /// @param rhs Mutex whose handle is exchanged.
        /// @return `*this` after the exchange.
        inline mutex& operator=(mutex&& rhs)
        {
            swap(_mutex, rhs._mutex);
            return *this;
        }

        /// @brief Blocks until the mutex is acquired.
        inline void lock()
        {
            mutex_detail::lock(_mutex);
        }

        /// @brief Attempts to acquire the mutex without blocking.
        /// @return True when the mutex was acquired.
        inline bool try_lock()
        {
            return mutex_detail::try_lock(_mutex);
        }

        /// @brief Releases the mutex.
        inline void unlock()
        {
            mutex_detail::unlock(_mutex);
        }

        /// @brief Returns the platform-native mutex handle address.
        /// @return Pointer to the native handle.
        inline auto* native_handle()
        {
            return &_mutex;
        }
    };

    /// @brief Busy-wait spin lock for short critical sections.
    class spin
    {
    private:
        /// @brief Atomic flag representing the spin lock state.
        atomic_flag _spin{};

    public:
        /// @brief Constructs an unlocked spin lock.
        inline spin() = default;

        /// @brief Spins until the flag can be acquired.
        inline void lock()
        {
            while (_spin.test_and_set(memory_order_acquire))
                ;
        }

        /// @brief Attempts to acquire the flag without waiting.
        /// @return True when the lock was acquired.
        inline bool try_lock()
        {
            return !_spin.test_and_set(memory_order_acquire);
        }

        /// @brief Releases the spin lock.
        inline void unlock()
        {
            _spin.clear(memory_order_release);
        }

        /// @brief Returns the address of the underlying atomic flag.
        /// @return Pointer to the native flag.
        inline auto* native_handle()
        {
            return &_spin;
        }
    };

    /// @brief Tag requesting deferred lock acquisition.
    struct dont_lock_t {};
    /// @brief Tag asserting that a mutex is already locked.
    struct adopt_lock_t {};

    /// @brief Deferred-lock construction tag.
    static constexpr auto dont_lock = dont_lock_t{};
    /// @brief Adopt-already-locked construction tag.
    static constexpr auto adopt_lock = adopt_lock_t{};

    /// @brief Movable RAII owner for a lockable synchronization primitive.
    /// @details Stores a mutex pointer and ownership bit. Destruction unlocks
    /// only when this guard owns the lock.
    /// @tparam Mutex Lockable primitive type.
    template <typename Mutex>
    class unique_lock
    {
    private:
        /// @brief Mutex guarded by this owner, or null for an empty guard.
        Mutex* _mutex;
        /// @brief Whether this guard currently owns the mutex.
        bool _is_locked;

    public:
        /// @brief Exchanges mutex pointers and ownership flags.
        /// @param lhs First guard.
        /// @param rhs Second guard.
        inline friend void swap(unique_lock& lhs, unique_lock& rhs)
        {
            hsd::swap(lhs._mutex, rhs._mutex);
            hsd::swap(lhs._is_locked, rhs._is_locked);
        }

        /// @brief Constructs an empty, unlocked guard.
        inline unique_lock()
            : _mutex{nullptr}, _is_locked{false} {}

        /// @brief Defers locking `mutex`.
        /// @param mutex Mutex retained without acquiring.
        inline unique_lock(dont_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false} {}

        /// @brief Adopts an already-held `mutex`.
        /// @param mutex Mutex asserted to be locked by this guard.
        inline unique_lock(adopt_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{true} {}

        /// @brief Locks `mutex` and adopts it on construction.
        /// @param mutex Mutex to acquire.
        inline unique_lock(Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false}
        {
            lock().unwrap();
        }

        inline unique_lock(const unique_lock&) = delete;
        inline unique_lock& operator=(const unique_lock&) = delete;

        /// @brief Transfers guard ownership from `other`.
        /// @param other Guard whose state is moved.
        inline unique_lock(unique_lock&& other)
            : unique_lock{}
        {
            swap(*this, other);
        }

        /// @brief Unlocks the owned mutex during destruction.
        inline ~unique_lock()
        {
            if (_is_locked == true)
                unlock().unwrap();
        }

        /// @brief Transfers guard ownership from `rhs`.
        /// @param rhs Guard whose state is moved.
        /// @return `*this` after assignment.
        inline unique_lock& operator=(unique_lock&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Acquires the guarded mutex.
        /// @return Empty success or an error when already locked.
        inline option_err<runtime_error> lock()
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

        /// @brief Releases the guarded mutex.
        /// @return Empty success or an error when not locked.
        inline option_err<runtime_error> unlock()
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
        
        /// @brief Returns the guarded mutex pointer.
        /// @return Mutex pointer, possibly null.
        inline Mutex* get_mutex() const
        {
            return _mutex;
        }

        /// @brief Reports whether this guard owns its mutex.
        /// @return True when the guard currently owns the lock.
        inline bool is_locked()
        {
            return _is_locked;
        }

        /// @brief Tests whether this guard owns `mutex`.
        /// @param mutex Mutex tested for ownership.
        /// @return True when the guard owns the supplied mutex.
        inline bool protects(Mutex* mutex)
        {
            return _is_locked && mutex == _mutex;
        }
    };

    /// @brief Movable RAII owner for shared lock acquisition.
    /// @details Stores a mutex pointer and ownership bit. Destruction releases
    /// a shared lock only when this guard owns one.
    /// @tparam Mutex Shared-lockable primitive type.
    template <typename Mutex>
    class shared_lock
    {
    private:
        /// @brief Mutex guarded by this owner, or null for an empty guard.
        Mutex* _mutex;
        /// @brief Whether this guard currently owns a shared lock.
        bool _is_locked;

    public:
        /// @brief Exchanges mutex pointers and ownership flags.
        /// @param lhs First guard.
        /// @param rhs Second guard.
        inline friend void swap(shared_lock& lhs, shared_lock& rhs)
        {
            hsd::swap(lhs._mutex, rhs._mutex);
            hsd::swap(lhs._is_locked, rhs._is_locked);
        }

        /// @brief Constructs an empty, unlocked shared guard.
        inline shared_lock()
            : _mutex{nullptr}, _is_locked{false}
        {}

        /// @brief Defers shared locking `mutex`.
        /// @param mutex Mutex retained without acquiring.
        inline shared_lock(dont_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false}
        {}

        /// @brief Adopts an already-held shared lock on `mutex`.
        /// @param mutex Mutex asserted to be shared-locked by this guard.
        inline shared_lock(adopt_lock_t, Mutex& mutex)
            : _mutex{&mutex}, _is_locked{true}
        {}

        /// @brief Acquires a shared lock on `mutex`.
        /// @param mutex Mutex to acquire in shared mode.
        inline shared_lock(Mutex& mutex)
            : _mutex{&mutex}, _is_locked{false}
        {
            lock().unwrap();
        }

        inline shared_lock(const shared_lock&) = delete;
        inline shared_lock& operator=(const shared_lock&) = delete;

        /// @brief Transfers shared guard ownership from `other`.
        /// @param other Guard whose state is moved.
        inline shared_lock(shared_lock&& other)
            : shared_lock{}
        {
            swap(*this, other);
        }

        /// @brief Releases the owned shared lock during destruction.
        inline ~shared_lock()
        {
            if (_is_locked == true)
                unlock().unwrap();
        }

        /// @brief Transfers shared guard ownership from `rhs`.
        /// @param rhs Guard whose state is moved.
        /// @return `*this` after assignment.
        inline shared_lock& operator=(shared_lock&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Acquires the guarded mutex in shared mode.
        /// @return Empty success or an error when already locked.
        inline option_err<runtime_error> lock()
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

        /// @brief Releases the guarded shared lock.
        /// @return Empty success or an error when not locked.
        inline option_err<runtime_error> unlock()
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

        /// @brief Returns the guarded mutex pointer.
        /// @return Mutex pointer, possibly null.
        inline Mutex* get_mutex() const
        {
            return &_mutex;
        }

        /// @brief Reports whether this guard owns a shared lock.
        /// @return True when the guard currently owns a shared lock.
        inline bool is_locked()
        {
            return _is_locked;
        }

        /// @brief Tests whether this guard owns `mutex`.
        /// @param mutex Mutex tested for ownership.
        /// @return True when the guard owns the supplied mutex.
        inline bool protects(Mutex* mutex)
        {
            return _is_locked && mutex == _mutex;
        }
    };

    /// @brief Constructs an owning unique_lock guard from a mutex pointer.
    /// @param mutex Mutex pointer to guard.
    /// @return Newly constructed owning guard.
    template <typename Mutex>
    unique_lock<Mutex> guard(Mutex* mutex)
    {
        return unique_lock<Mutex>(*mutex);
    }

    /// @brief Constructs a deferred unique_lock guard from a mutex pointer.
    /// @param mutex Mutex pointer to retain without locking.
    /// @return Newly constructed deferred guard.
    template <typename Mutex>
    unique_lock<Mutex> guard(dont_lock_t, Mutex* mutex)
    {
        return unique_lock<Mutex>(dont_lock, *mutex);
    }
} // namespace hsd
