#pragma once

#include "Concepts.hpp"

#include <stddef.h>
#include <stdint.h>

namespace hsd
{
    enum class memory_order : i32
    {
        relaxed = __ATOMIC_RELAXED,
        consume = __ATOMIC_CONSUME,
        acquire = __ATOMIC_ACQUIRE,
        release = __ATOMIC_RELEASE,
        acq_rel = __ATOMIC_ACQ_REL,
        seq_cst = __ATOMIC_SEQ_CST
    };

    static constexpr auto memory_order_relaxed = memory_order::relaxed;
    static constexpr auto memory_order_consume = memory_order::consume;
    static constexpr auto memory_order_acquire = memory_order::acquire;
    static constexpr auto memory_order_release = memory_order::release;
    static constexpr auto memory_order_acq_rel = memory_order::acq_rel;
    static constexpr auto memory_order_seq_cst = memory_order::seq_cst;

    namespace atomic_detail
    {
        template <typename T>
        concept valid_atomic_type =
            std::is_trivially_copyable_v<T> &&
            CopyConstructible<T> && 
            MoveConstructible<T> &&
            CopyAssignable<T> 	 && 
            MoveAssignable<T>;

        template <typename T, bool IsRef>
        struct storage;

        template <typename T>
        struct storage<T, false>
        {
            T _value;
        };

        template <typename T>
        struct storage<T, true>
        {
            T* _ptr;
        };

        template <typename T>
        struct difference_type_provider {};

        template <typename T>
        requires(is_number<T>::value || is_pointer<T>::value)
        struct difference_type_provider<T>
        {
            using difference_type = conditional_t<
                is_number<T>::value,T, pdiff
            >;
        };

        template <valid_atomic_type T, bool IsRef>
        struct atomic_impl
            : difference_type_provider<T>
        {
            static constexpr bool is_always_lock_free =
                __atomic_always_lock_free(sizeof(T), nullptr);

            using value_type = T;

            constexpr atomic_impl() requires(!IsRef)
                : _stor{}
            {}

            constexpr atomic_impl() requires(IsRef)
                : _stor{}
            {}

            constexpr atomic_impl(T desired) requires(!IsRef)
                : _stor{desired}
            {}

            explicit constexpr  atomic_impl(T &desired) requires(IsRef)
                : _stor{&desired}
            {}

            atomic_impl(const atomic_impl&) requires(!IsRef) = delete;
            atomic_impl(const atomic_impl&) requires(IsRef) = default;
            atomic_impl& operator=(const atomic_impl&) = delete;
            atomic_impl& operator=(const atomic_impl&) volatile = delete;

            constexpr T operator=(T desired)
            {
                store(desired);
                return desired;
            }

            constexpr T operator=(T desired) volatile
            {
                store(desired);
                return desired;
            }

            constexpr bool is_lock_free() const
            {
                return __atomic_is_lock_free(sizeof(T), _data_ptr());
            }

            constexpr bool is_lock_free() const volatile
            {
                return __atomic_is_lock_free(sizeof(T), _data_ptr());
            }

            constexpr void store(
                T desired, memory_order order = memory_order_seq_cst)
            {
                __atomic_store(_data_ptr(), &desired, static_cast<i32>(order));
            }

            constexpr void store(
                T desired, memory_order order = memory_order_seq_cst) volatile
            {
                __atomic_store(_data_ptr(), &desired, static_cast<i32>(order));
            }

            constexpr T load(
                memory_order order = memory_order_seq_cst) const
            {
                T _tmp;
                __atomic_load(_data_ptr(), &_tmp, static_cast<i32>(order));
                return _tmp;
            }

            constexpr T load(
                memory_order order = memory_order_seq_cst) const volatile
            {
                T _tmp;
                __atomic_load(_data_ptr(), &_tmp, static_cast<i32>(order));
                return _tmp;
            }

            constexpr operator T() const
            {
                return load();
            }

            constexpr operator T() const volatile
            {
                return load();
            }

            constexpr T exchange(T desired, memory_order order = memory_order_seq_cst)
            {
                T _ret;
                __atomic_exchange(_data_ptr(), &desired, &_ret, static_cast<i32>(order));
                return _ret;
            }

            constexpr T exchange(T desired, memory_order order = memory_order_seq_cst) volatile
            {
                T _ret;
                __atomic_exchange(_data_ptr(), &desired, &_ret, static_cast<i32>(order));
                return _ret;
            }

            constexpr bool compare_exchange_weak(
                T& expected, T desired,
                memory_order success, 
                memory_order failure)
            {
                return __atomic_compare_exchange(
                    _data_ptr(), &expected, &desired, true,
                    static_cast<i32>(success), static_cast<i32>(failure)
                );
            }

            constexpr bool compare_exchange_weak(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst)
            {
                return compare_exchange_weak(expected, desired, order, order);
            }

            constexpr bool compare_exchange_weak(
                T& expected, T desired,
                memory_order success, 
                memory_order failure) volatile
            {
                return __atomic_compare_exchange(
                    _data_ptr(), &expected, &desired, true,
                    static_cast<i32>(success), static_cast<i32>(failure)
                );
            }

            constexpr bool compare_exchange_weak(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst) volatile
            {
                return compare_exchange_weak(expected, desired, order, order);
            }

            constexpr bool compare_exchange_strong(
                T& expected, T desired,
                memory_order success, 
                memory_order failure)
            {
                return __atomic_compare_exchange(
                    _data_ptr(), &expected, &desired, false,
                    static_cast<i32>(success), static_cast<i32>(failure)
                );
            }

            constexpr bool compare_exchange_strong(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst)
            {
                return compare_exchange_strong(expected, desired, order, order);
            }

            constexpr bool compare_exchange_strong(
                T& expected, T desired,
                memory_order success, 
                memory_order failure) volatile
            {
                return __atomic_compare_exchange(
                    _data_ptr(), &expected, &desired, false,
                    static_cast<i32>(success), static_cast<i32>(failure)
                );
            }

            constexpr bool compare_exchange_strong(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst) volatile
            {
                return compare_exchange_strong(expected, desired, order, order);
            }

            constexpr T fetch_add(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_number<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_add(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_number<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_add(
                pdiff arg, memory_order order = memory_order_seq_cst)
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            constexpr T fetch_add(
                pdiff arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            constexpr T fetch_sub(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_number<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_sub(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_number<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_sub(
                pdiff arg, memory_order order = memory_order_seq_cst)
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            constexpr T fetch_sub(
                pdiff arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            constexpr T fetch_and(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_and(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_and(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_and(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_or(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_or(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_or(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_or(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_xor(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_xor(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T fetch_xor(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_xor(_data_ptr(), arg, static_cast<i32>(order));
            }

            constexpr T operator++()
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1) + 1;
            }

            constexpr T operator++(i32)
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1);
            }

            constexpr T operator++() volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1) + 1;
            }

            constexpr T operator++(i32) volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1);
            }

            constexpr T operator--()
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1) - 1;
            }

            constexpr T operator--(i32)
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1);
            }

            constexpr T operator--() volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1) - 1;
            }

            constexpr T operator--(i32) volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1);
            }

            constexpr T operator+=(T arg)
            requires (is_number<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            constexpr T operator+=(T arg) volatile
            requires (is_number<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            constexpr T operator+=(pdiff arg)
            requires (is_pointer<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            constexpr T operator+=(pdiff arg) volatile
            requires (is_pointer<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            constexpr T operator-=(T arg)
            requires (is_number<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            constexpr T operator-=(T arg) volatile
            requires (is_number<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            constexpr T operator-=(pdiff arg)
            requires (is_pointer<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            constexpr T operator-=(pdiff arg) volatile
            requires (is_pointer<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            constexpr T operator&=(T arg)
            requires (is_integral<T>::value)
            {
                return fetch_and(arg) & arg;
            }

            constexpr T operator&=(T arg) volatile
            requires (is_integral<T>::value)
            {
                return fetch_and(arg) & arg;
            }

            constexpr T operator|=(T arg)
            requires (is_integral<T>::value)
            {
                return fetch_or(arg) | arg;
            }

            constexpr T operator|=(T arg) volatile
            requires (is_integral<T>::value)
            {
                return fetch_or(arg) | arg;
            }

            constexpr T operator^=(T arg)
            requires (is_integral<T>::value)
            {
                return fetch_xor(arg) ^ arg;
            }

            constexpr T operator^=(T arg) volatile
            requires (is_integral<T>::value)
            {
                return fetch_xor(arg) ^ arg;
            }

        private:
            constexpr T* _data_ptr()
            {
                if constexpr (IsRef)
                {
                    return _stor._ptr;
                }
                else
                {
                    return &_stor._value;
                }
            }

            constexpr const T* _data_ptr() const
            {
                if constexpr (IsRef)
                {
                    return _stor._ptr;
                }
                else
                {
                    return &_stor._value;
                }
            }

            storage<T, IsRef> _stor;
        };

    } // namespace atomic_detail

    extern "C"
    {
        inline void atomic_thread_fence(memory_order order)
        {
            __atomic_thread_fence(static_cast<i32>(order));
        }

        inline void atomic_signal_fence(memory_order order)
        {
            __atomic_signal_fence(static_cast<i32>(order));
        }
    }

    // qookie: This is what libstdc++ does
    template <typename T>
    static constexpr T kill_dependency(T y)
    {
        T _tmp{y};
        return _tmp;
    }

    template <typename T>
    using atomic = atomic_detail::atomic_impl<T, false>;

    using atomic_bool = atomic<bool>;
    using atomic_char = atomic<char>;
    using atomic_uchar = atomic<uchar>;
    using atomic_schar = atomic<schar>;
    using atomic_short = atomic<i16>;
    using atomic_ushort = atomic<u16>;
    using atomic_i32 = atomic<i32>;
    using atomic_u32 = atomic<u32>;
    using atomic_long = atomic<long>;
    using atomic_ulong = atomic<ulong>;
    using atomic_i64 = atomic<i64>;
    using atomic_u64 = atomic<u64>;
    using atomic_char8 = atomic<char8>;
    using atomic_char16 = atomic<char16>;
    using atomic_char32 = atomic<char32>;
    using atomic_wchar = atomic<wchar>;
    using atomic_iptr = atomic<iptr>;
    using atomic_uptr = atomic<uptr>;
    using atomic_isize = atomic<isize>;
    using atomic_usize = atomic<usize>;
    using atomic_pdiff = atomic<pdiff>;
    using atomic_imax = atomic<imax>;
    using atomic_umax = atomic<umax>;

    template <typename T>
    using atomic_ref = atomic_detail::atomic_impl<T, true>;

    template <typename T>
    static inline void atomic_store(atomic<T>* obj,
        typename atomic<T>::value_type dest)
    {
        obj->store(dest);
    }

    template <typename T>
    static inline void atomic_store(volatile atomic<T>* obj,
        typename atomic<T>::value_type dest)
    {
        obj->store(dest);
    }

    template <typename T>
    static inline void atomic_store_explicit(atomic<T>* obj,
        typename atomic<T>::value_type dest, memory_order order)
    {
        obj->store(dest, order);
    }

    template <typename T>
    static inline void atomic_store_explicit(volatile atomic<T>* obj,
        typename atomic<T>::value_type dest, memory_order order)
    {
        obj->store(dest, order);
    }

    template <typename T>
    static inline T atomic_load(atomic<T>* obj)
    {
        return obj->load();
    }

    template <typename T>
    static inline T atomic_load(volatile atomic<T>* obj)
    {
        return obj->load();
    }

    template <typename T>
    static inline T atomic_load_explicit(
        atomic<T>* obj, memory_order order)
    {
        return obj->load(order);
    }

    template <typename T>
    static inline T atomic_load_explicit(
        volatile atomic<T>* obj, memory_order order)
    {
        return obj->load(order);
    }

    template <typename T>
    static inline T atomic_exchange(
        atomic<T>* obj, 
        typename atomic<T>::value_type dest)
    {
        return obj->exchange(dest);
    }

    template <typename T>
    static inline T atomic_exchange(
        volatile atomic<T> *obj, 
        typename atomic<T>::value_type dest)
    {
        return obj->exchange(dest);
    }

    template <typename T>
    static inline T atomic_exchange_explicit(
        atomic<T>* obj,
        typename atomic<T>::value_type dest, 
        memory_order order)
    {
        return obj->exchange(dest, order);
    }

    template <typename T>
    static inline T atomic_exchange_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type dest, 
        memory_order order)
    {
        return obj->exchange(dest, order);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_weak(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_weak(*expected, desired);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_weak(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_weak(*expected, desired);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_strong(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_strong(*expected, desired);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_strong(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_strong(*expected, desired);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_weak_explicit(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_weak(*expected, desired, succ, fail);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_weak_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_weak(*expected, desired, succ, fail);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_strong_explicit(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_strong(*expected, desired, succ, fail);
    }

    template <typename T>
    static inline bool atomic_compare_exchange_strong_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_strong(*expected, desired, succ, fail);
    }

    template <typename T>
    static inline T atomic_fetch_add(
        atomic<T>* obj, 
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_add(arg);
    }

    template <typename T>
    static inline T atomic_fetch_add(
        volatile atomic<T>* obj, 
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_add(arg);
    }

    template <typename T>
    static inline T atomic_fetch_add_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_add(arg, order);
    }

    template <typename T>
    static inline T atomic_fetch_add_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_add(arg, order);
    }

    template <typename T>
    static inline T atomic_fetch_and(
        atomic<T>* obj, 
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_and(arg);
    }

    template <typename T>
    static inline T atomic_fetch_and(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_and(arg);
    }

    template <typename T>
    static inline T atomic_fetch_and_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_and(arg, order);
    }

    template <typename T>
    static inline T atomic_fetch_and_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_and(arg, order);
    }

    template <typename T>
    static inline T atomic_fetch_or(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_or(arg);
    }

    template <typename T>
    static inline T atomic_fetch_or(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_or(arg);
    }

    template <typename T>
    static inline T atomic_fetch_or_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg,
         memory_order order)
    {
        return obj->fetch_or(arg, order);
    }

    template <typename T>
    static inline T atomic_fetch_or_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_or(arg, order);
    }

    template <typename T>
    static inline T atomic_fetch_xor(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_xor(arg);
    }

    template <typename T>
    static inline T atomic_fetch_xor(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_xor(arg);
    }

    template <typename T>
    static inline T atomic_fetch_xor_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_xor(arg, order);
    }

    template <typename T>
    static inline T atomic_fetch_xor_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg,
         memory_order order)
    {
        return obj->fetch_xor(arg, order);
    }

    struct atomic_flag
    {
        constexpr atomic_flag()
            : _flag{}
        {}

        constexpr atomic_flag(const atomic_flag&) = delete;
        constexpr atomic_flag& operator=(const atomic_flag&) = delete;
        constexpr atomic_flag& operator=(const atomic_flag&) volatile = delete;

        inline void clear(memory_order order = memory_order_seq_cst)
        {
            __atomic_clear(&_flag, static_cast<i32>(order));
        }

        inline void clear(memory_order order = memory_order_seq_cst) volatile
        {
            __atomic_clear(&_flag, static_cast<i32>(order));
        }

        inline bool test_and_set(memory_order order = memory_order_seq_cst)
        {
            return __atomic_test_and_set(&_flag, static_cast<i32>(order));
        }

        inline bool test_and_set(memory_order order = memory_order_seq_cst) volatile
        {
            return __atomic_test_and_set(&_flag, static_cast<i32>(order));
        }

        inline bool test(memory_order order = memory_order_seq_cst) const
        {
            return __atomic_load_n(&_flag, static_cast<i32>(order));
        }

        inline bool test(memory_order order = memory_order_seq_cst) const volatile
        {
            return __atomic_load_n(&_flag, static_cast<i32>(order));
        }

    private:
        uchar _flag;
    };

    static inline bool atomic_flag_test_and_set(atomic_flag* flag)
    {
        return flag->test_and_set();
    }

    static inline bool atomic_flag_test_and_set(volatile atomic_flag* flag)
    {
        return flag->test_and_set();
    }

    static inline bool atomic_flag_test_and_set_explicit(
        atomic_flag* flag, memory_order order)
    {
        return flag->test_and_set(order);
    }

    static inline bool atomic_flag_test_and_set_explicit(
        volatile atomic_flag* flag, memory_order order)
    {
        return flag->test_and_set(order);
    }

    static inline void atomic_flag_clear(atomic_flag* flag)
    {
        flag->clear();
    }

    static inline void atomic_flag_clear(volatile atomic_flag* flag)
    {
        flag->clear();
    }

    static inline void atomic_flag_clear_explicit(
        atomic_flag* flag, memory_order order)
    {
        flag->clear(order);
    }

    static inline void atomic_flag_clear_explicit(
        volatile atomic_flag* flag, memory_order order)
    {
        flag->clear(order);
    }

    static inline bool atomic_flag_test(atomic_flag* flag)
    {
        return flag->test();
    }

    static inline bool atomic_flag_test(volatile atomic_flag* flag)
    {
        return flag->test();
    }

    static inline bool atomic_flag_test_explicit(
        atomic_flag* flag, memory_order order)
    {
        return flag->test(order);
    }

    static inline bool atomic_flag_test_explicit(
        volatile atomic_flag* flag, memory_order order)
    {
        return flag->test(order);
    }
} // namespace hsd