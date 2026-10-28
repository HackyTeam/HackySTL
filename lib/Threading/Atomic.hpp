#pragma once

/// @file Atomic.hpp
/// @brief Atomic values, flags, memory orders, and atomic operations.
/// @details Wraps compiler atomic primitives with project aliases for loads,
/// stores, exchange, compare/exchange, flags, and arithmetic operations.

#include "../Base/Concepts.hpp"

#include <stddef.h>
#include <stdint.h>

namespace hsd
{
    /// @brief Memory-order constraints accepted by atomic operations.
    enum class memory_order : i32
    {
        relaxed = __ATOMIC_RELAXED,
        consume = __ATOMIC_CONSUME,
        acquire = __ATOMIC_ACQUIRE,
        release = __ATOMIC_RELEASE,
        acq_rel = __ATOMIC_ACQ_REL,
        seq_cst = __ATOMIC_SEQ_CST
    };

    /// @brief Relaxed ordering constant.
    static constexpr auto memory_order_relaxed = memory_order::relaxed;
    /// @brief Consume ordering constant.
    static constexpr auto memory_order_consume = memory_order::consume;
    /// @brief Acquire ordering constant.
    static constexpr auto memory_order_acquire = memory_order::acquire;
    /// @brief Release ordering constant.
    static constexpr auto memory_order_release = memory_order::release;
    /// @brief Acquire-release ordering constant.
    static constexpr auto memory_order_acq_rel = memory_order::acq_rel;
    /// @brief Sequentially consistent ordering constant.
    static constexpr auto memory_order_seq_cst = memory_order::seq_cst;

    namespace atomic_detail
    {
        template <typename T>
        concept valid_atomic_type =
            std::is_trivially_copyable_v<T> &&
            CopyConstructible<T>            && 
            MoveConstructible<T>            &&
            CopyAssignable<T> 	            && 
            MoveAssignable<T>;

        /// @brief Selects inline value storage or referenced storage.
        /// @tparam T Atomic value type.
        /// @tparam IsRef Whether the atomic aliases an existing object.
        template <typename T, bool IsRef>
        struct storage;

        /// @brief Storage for an owning atomic value.
        template <typename T>
        struct storage<T, false>
        {
            /// @brief Value modified by the compiler atomic builtins.
            T _value;
        };

        /// @brief Storage for an atomic reference.
        template <typename T>
        struct storage<T, true>
        {
            /// @brief Address of the externally owned value.
            T* _ptr;
        };

        /// @brief Supplies the arithmetic difference type for fetch operations.
        template <typename T>
        struct difference_type_provider {};

        /// @brief Adds the arithmetic difference type for numeric and pointer atomics.
        template <typename T>
        requires(is_number<T>::value || is_pointer<T>::value)
        struct difference_type_provider<T>
        {
            /// @brief Difference accepted by fetch-add and fetch-sub.
            using difference_type = conditional_t<
                is_number<T>::value,T, pdiff
            >;
        };

        /// @brief Common implementation for owning atomics and atomic references.
        /// @details The implementation delegates synchronization to GCC atomic
        /// builtins. An owning instance stores `T` inline; a reference instance
        /// stores only the address supplied at construction.
        /// @tparam T Trivially copyable value type accepted by the atomic builtins.
        /// @tparam IsRef Selects reference semantics when true.
        template <valid_atomic_type T, bool IsRef>
        struct atomic_impl
            : difference_type_provider<T>
        {
            /// @brief Compile-time lock-free guarantee for this value size.
            static constexpr bool is_always_lock_free =
                __atomic_always_lock_free(sizeof(T), nullptr);

            /// @brief Value type manipulated by this atomic.
            using value_type = T;

            /// @brief Constructs an owning atomic with value initialization.
            constexpr atomic_impl() requires(!IsRef)
                : _stor{}
            {}

            /// @brief Constructs an atomic reference with a null target.
            constexpr atomic_impl() requires(IsRef)
                : _stor{}
            {}

            /// @brief Constructs an owning atomic with `desired`.
            /// @param desired Initial value.
            constexpr atomic_impl(T desired) requires(!IsRef)
                : _stor{desired}
            {}

            /// @brief Makes an atomic reference to `desired`.
            /// @param desired Existing object whose storage is accessed atomically.
            explicit constexpr  atomic_impl(T &desired) requires(IsRef)
                : _stor{&desired}
            {}

            atomic_impl(const atomic_impl&) requires(!IsRef) = delete;
            /// @brief Copies an atomic reference's target address.
            atomic_impl(const atomic_impl&) requires(IsRef) = default;
            atomic_impl& operator=(const atomic_impl&) = delete;
            atomic_impl& operator=(const atomic_impl&) volatile = delete;

            /// @brief Atomically stores `desired` and returns it.
            /// @param desired Value to store.
            /// @return The value stored.
            constexpr T operator=(T desired)
            {
                store(desired);
                return desired;
            }

            /// @brief Volatile atomic assignment using the default order.
            /// @param desired Value to store.
            /// @return The value stored.
            constexpr T operator=(T desired) volatile
            {
                store(desired);
                return desired;
            }

            /// @brief Returns whether the addressed object is lock-free.
            /// @return Whether the object is lock-free.
            constexpr bool is_lock_free() const
            {
                return __atomic_is_lock_free(sizeof(T), _data_ptr());
            }

            /// @brief Volatile lock-free query for the addressed object.
            /// @return Whether the object is lock-free.
            constexpr bool is_lock_free() const volatile
            {
                return __atomic_is_lock_free(sizeof(T), _data_ptr());
            }

            /// @brief Atomically stores a value with the selected memory order.
            /// @param desired Value written to the object.
            /// @param order Ordering applied to the store.
            constexpr void store(
                T desired, memory_order order = memory_order_seq_cst)
            {
                __atomic_store(_data_ptr(), &desired, static_cast<i32>(order));
            }

            /// @brief Volatile atomic store with the selected memory order.
            /// @param desired Value written to the object.
            /// @param order Ordering applied to the store.
            constexpr void store(
                T desired, memory_order order = memory_order_seq_cst) volatile
            {
                __atomic_store(_data_ptr(), &desired, static_cast<i32>(order));
            }

            /// @brief Atomically loads a value with the selected memory order.
            /// @param order Ordering applied to the load.
            /// @return The value observed by the load.
            constexpr T load(
                memory_order order = memory_order_seq_cst) const
            {
                T _tmp;
                __atomic_load(_data_ptr(), &_tmp, static_cast<i32>(order));
                return _tmp;
            }

            /// @brief Volatile atomic load with the selected memory order.
            /// @param order Ordering applied to the load.
            /// @return The value observed by the load.
            constexpr T load(
                memory_order order = memory_order_seq_cst) const volatile
            {
                T _tmp;
                __atomic_load(_data_ptr(), &_tmp, static_cast<i32>(order));
                return _tmp;
            }

            /// @brief Loads the current value using sequential consistency.
            /// @return The value observed by the load.
            constexpr operator T() const
            {
                return load();
            }

            /// @brief Converts a volatile atomic to its loaded value.
            /// @return The value observed by a sequentially consistent load.
            constexpr operator T() const volatile
            {
                return load();
            }

            /// @brief Atomically replaces the value and returns the old value.
            /// @param desired Value written to the object.
            /// @param order Ordering applied to the exchange.
            /// @return The value present before the exchange.
            constexpr T exchange(T desired, memory_order order = memory_order_seq_cst)
            {
                T _ret;
                __atomic_exchange(_data_ptr(), &desired, &_ret, static_cast<i32>(order));
                return _ret;
            }

            /// @brief Volatile atomic exchange returning the old value.
            /// @param desired Replacement value.
            /// @param order Ordering applied to the exchange.
            /// @return The value present before the exchange.
            constexpr T exchange(T desired, memory_order order = memory_order_seq_cst) volatile
            {
                T _ret;
                __atomic_exchange(_data_ptr(), &desired, &_ret, static_cast<i32>(order));
                return _ret;
            }

            /// @brief Weak compare/exchange with separate success and failure orders.
            /// @param expected Expected old value; overwritten with the observed value on failure.
            /// @param desired Replacement value on a successful comparison.
            /// @param success Ordering used when the exchange succeeds.
            /// @param failure Ordering used when the comparison fails.
            /// @return Whether the exchange took place.
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

            /// @brief Weak compare/exchange using one order for both outcomes.
            /// @brief Volatile weak compare/exchange with one memory order.
            /// @param expected Expected value, replaced with the observation on failure.
            /// @param desired Replacement value on success.
            /// @param order Memory order used for both outcomes.
            /// @return Whether the exchange succeeded.
            constexpr bool compare_exchange_weak(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst)
            {
                return compare_exchange_weak(expected, desired, order, order);
            }

            /// @brief Volatile weak compare/exchange with separate memory orders.
            /// @param expected Expected value, replaced with the observation on failure.
            /// @param desired Replacement value on success.
            /// @param success Success memory order.
            /// @param failure Failure memory order.
            /// @return Whether the exchange succeeded.
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

            /// @brief Volatile weak compare/exchange with one memory order.
            /// @param expected Expected value, replaced with the observation on failure.
            /// @param desired Replacement value on success.
            /// @param order Memory order used for both outcomes.
            /// @return Whether the exchange succeeded.
            constexpr bool compare_exchange_weak(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst) volatile
            {
                return compare_exchange_weak(expected, desired, order, order);
            }

            /// @brief Strong compare/exchange with separate success and failure orders.
            /// @param expected Expected old value; overwritten with the observed value on failure.
            /// @param desired Replacement value on a successful comparison.
            /// @param success Ordering used when the exchange succeeds.
            /// @param failure Ordering used when the comparison fails.
            /// @return Whether the exchange took place.
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

            /// @brief Strong compare/exchange using one order for both outcomes.
            /// @brief Volatile strong compare/exchange with one memory order.
            /// @param expected Expected value, replaced with the observation on failure.
            /// @param desired Replacement value on success.
            /// @param order Memory order used for both outcomes.
            /// @return Whether the exchange succeeded.
            constexpr bool compare_exchange_strong(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst)
            {
                return compare_exchange_strong(expected, desired, order, order);
            }

            /// @brief Volatile strong compare/exchange with separate memory orders.
            /// @param expected Expected value, replaced with the observation on failure.
            /// @param desired Replacement value on success.
            /// @param success Success memory order.
            /// @param failure Failure memory order.
            /// @return Whether the exchange succeeded.
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

            /// @brief Volatile strong compare/exchange with one memory order.
            /// @param expected Expected value, replaced with the observation on failure.
            /// @param desired Replacement value on success.
            /// @param order Memory order used for both outcomes.
            /// @return Whether the exchange succeeded.
            constexpr bool compare_exchange_strong(
                T& expected, T desired,
                memory_order order = memory_order_seq_cst) volatile
            {
                return compare_exchange_strong(expected, desired, order, order);
            }

            /// @brief Atomically adds to an arithmetic value and returns the old value.
            /// @param arg Increment applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the addition.
            constexpr T fetch_add(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_number<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Volatile arithmetic fetch-add returning the old value.
            /// @param arg Increment applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the addition.
            constexpr T fetch_add(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_number<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Pointer fetch-add using element distances.
            /// @param arg Number of pointed-to elements to advance.
            /// @param order Ordering applied to the operation.
            /// @return The pointer present before the addition.
            constexpr T fetch_add(
                pdiff arg, memory_order order = memory_order_seq_cst)
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            /// @brief Volatile pointer fetch-add using element distances.
            /// @param arg Number of pointed-to elements to advance.
            /// @param order Ordering applied to the operation.
            /// @return The pointer present before the addition.
            constexpr T fetch_add(
                pdiff arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_add(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            /// @brief Atomically subtracts from an arithmetic value and returns the old value.
            /// @param arg Decrement applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the subtraction.
            constexpr T fetch_sub(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_number<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Volatile arithmetic fetch-sub returning the old value.
            /// @param arg Decrement applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the subtraction.
            constexpr T fetch_sub(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_number<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Pointer fetch-sub using element distances.
            /// @param arg Number of pointed-to elements to retreat.
            /// @param order Ordering applied to the operation.
            /// @return The pointer present before the subtraction.
            constexpr T fetch_sub(
                pdiff arg, memory_order order = memory_order_seq_cst)
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            /// @brief Volatile pointer fetch-sub using element distances.
            /// @param arg Number of pointed-to elements to retreat.
            /// @param order Ordering applied to the operation.
            /// @return The pointer present before the subtraction.
            constexpr T fetch_sub(
                pdiff arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_pointer<T>::value)
            {
                return __atomic_fetch_sub(_data_ptr(),
                    arg * sizeof(remove_pointer_t<T>),
                    static_cast<i32>(order)
                );
            }

            /// @brief Atomically applies bitwise AND and returns the old value.
            /// @param arg Mask applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the operation.
            constexpr T fetch_and(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_and(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Volatile atomic bitwise AND returning the old value.
            /// @param arg Mask applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the operation.
            constexpr T fetch_and(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_and(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Atomically applies bitwise OR and returns the old value.
            /// @param arg Mask applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the operation.
            constexpr T fetch_or(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_or(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Volatile atomic bitwise OR returning the old value.
            /// @param arg Mask applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the operation.
            constexpr T fetch_or(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_or(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Atomically applies bitwise XOR and returns the old value.
            /// @param arg Mask applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the operation.
            constexpr T fetch_xor(
                T arg, memory_order order = memory_order_seq_cst)
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_xor(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Volatile atomic bitwise XOR returning the old value.
            /// @param arg Mask applied to the value.
            /// @param order Ordering applied to the operation.
            /// @return The value present before the operation.
            constexpr T fetch_xor(
                T arg, memory_order order = memory_order_seq_cst) volatile
            requires (is_integral<T>::value)
            {
                return __atomic_fetch_xor(_data_ptr(), arg, static_cast<i32>(order));
            }

            /// @brief Atomically increments and returns the new value.
            /// @return The incremented value.
            constexpr T operator++()
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1) + 1;
            }

            /// @brief Atomically increments and returns the old value.
            /// @return The value before incrementing.
            constexpr T operator++(i32)
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1);
            }

            /// @brief Volatile atomic prefix increment returning the new value.
            /// @return The incremented value.
            constexpr T operator++() volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1) + 1;
            }

            /// @brief Volatile atomic postfix increment returning the old value.
            /// @return The value before incrementing.
            constexpr T operator++(i32) volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_add(1);
            }

            /// @brief Atomically decrements and returns the new value.
            /// @return The decremented value.
            constexpr T operator--()
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1) - 1;
            }

            /// @brief Atomically decrements and returns the old value.
            /// @return The value before decrementing.
            constexpr T operator--(i32)
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1);
            }

            /// @brief Volatile atomic prefix decrement returning the new value.
            /// @return The decremented value.
            constexpr T operator--() volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1) - 1;
            }

            /// @brief Volatile atomic postfix decrement returning the old value.
            /// @return The value before decrementing.
            constexpr T operator--(i32) volatile
            requires (is_integral<T>::value || is_pointer<T>::value)
            {
                return fetch_sub(1);
            }

            /// @brief Atomically adds an arithmetic value and returns the result.
            /// @param arg Arithmetic increment.
            /// @return The value after addition.
            constexpr T operator+=(T arg)
            requires (is_number<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            /// @brief Volatile atomic arithmetic addition.
            /// @param arg Arithmetic increment.
            /// @return The value after addition.
            constexpr T operator+=(T arg) volatile
            requires (is_number<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            /// @brief Atomically advances a pointer by element distance.
            /// @param arg Number of elements to advance.
            /// @return The pointer after addition.
            constexpr T operator+=(pdiff arg)
            requires (is_pointer<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            /// @brief Volatile atomic pointer addition.
            /// @param arg Number of elements to advance.
            /// @return The pointer after addition.
            constexpr T operator+=(pdiff arg) volatile
            requires (is_pointer<T>::value)
            {
                return fetch_add(arg) + arg;
            }

            /// @brief Atomically subtracts an arithmetic value and returns the result.
            /// @param arg Arithmetic decrement.
            /// @return The value after subtraction.
            constexpr T operator-=(T arg)
            requires (is_number<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            /// @brief Volatile atomic arithmetic subtraction.
            /// @param arg Arithmetic decrement.
            /// @return The value after subtraction.
            constexpr T operator-=(T arg) volatile
            requires (is_number<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            /// @brief Atomically retreats a pointer by element distance.
            /// @param arg Number of elements to retreat.
            /// @return The pointer after subtraction.
            constexpr T operator-=(pdiff arg)
            requires (is_pointer<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            /// @brief Volatile atomic pointer subtraction.
            /// @param arg Number of elements to retreat.
            /// @return The pointer after subtraction.
            constexpr T operator-=(pdiff arg) volatile
            requires (is_pointer<T>::value)
            {
                return fetch_sub(arg) - arg;
            }

            /// @brief Atomically applies a bitwise AND and returns the result.
            /// @param arg Integral mask.
            /// @return The value after the operation.
            constexpr T operator&=(T arg)
            requires (is_integral<T>::value)
            {
                return fetch_and(arg) & arg;
            }

            /// @brief Volatile atomic bitwise AND assignment.
            /// @param arg Integral mask.
            /// @return The value after the operation.
            constexpr T operator&=(T arg) volatile
            requires (is_integral<T>::value)
            {
                return fetch_and(arg) & arg;
            }

            /// @brief Atomically applies a bitwise OR and returns the result.
            /// @param arg Integral mask.
            /// @return The value after the operation.
            constexpr T operator|=(T arg)
            requires (is_integral<T>::value)
            {
                return fetch_or(arg) | arg;
            }

            /// @brief Volatile atomic bitwise OR assignment.
            /// @param arg Integral mask.
            /// @return The value after the operation.
            constexpr T operator|=(T arg) volatile
            requires (is_integral<T>::value)
            {
                return fetch_or(arg) | arg;
            }

            /// @brief Atomically applies a bitwise XOR and returns the result.
            /// @param arg Integral mask.
            /// @return The value after the operation.
            constexpr T operator^=(T arg)
            requires (is_integral<T>::value)
            {
                return fetch_xor(arg) ^ arg;
            }

            /// @brief Volatile atomic bitwise XOR assignment.
            /// @param arg Integral mask.
            /// @return The value after the operation.
            constexpr T operator^=(T arg) volatile
            requires (is_integral<T>::value)
            {
                return fetch_xor(arg) ^ arg;
            }

        private:
            /// @brief Resolves the address passed to the compiler atomic builtins.
            /// @return Address of owned storage or the referenced object.
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

            /// @brief Const address resolver for atomic loads and lock queries.
            /// @return Address of owned storage or the referenced object.
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

            /// @brief Owns the value or points at the referenced value.
            storage<T, IsRef> _stor;
        };

    } // namespace atomic_detail

    /// @brief Emits a hardware/compiler thread fence with `order` semantics.
    /// @param order Memory ordering imposed by the fence.
    extern "C"
    {
        inline void atomic_thread_fence(memory_order order)
        {
            __atomic_thread_fence(static_cast<i32>(order));
        }

        /// @brief Emits a compiler-only signal fence with `order` semantics.
        /// @param order Memory ordering imposed by the fence.
        inline void atomic_signal_fence(memory_order order)
        {
            __atomic_signal_fence(static_cast<i32>(order));
        }
    }

    // qookie: This is what libstdc++ does
    /// @brief Prevents dependency analysis from propagating through a value copy.
    /// @param y Value whose dependency is discarded.
    /// @return A copy of `y` with no source dependency visible to the compiler.
    template <typename T>
    static constexpr T kill_dependency(T y)
    {
        T _tmp{y};
        return _tmp;
    }

    /// @brief Owning atomic value with compiler-provided synchronization.
    template <typename T>
    using atomic = atomic_detail::atomic_impl<T, false>;

    /// @brief Atomic boolean alias.
    using atomic_bool = atomic<bool>;
    /// @brief Atomic narrow character alias.
    using atomic_char = atomic<char>;
    /// @brief Atomic unsigned narrow character alias.
    using atomic_uchar = atomic<uchar>;
    /// @brief Atomic signed narrow character alias.
    using atomic_schar = atomic<schar>;
    /// @brief Atomic short integer alias.
    using atomic_short = atomic<i16>;
    /// @brief Atomic unsigned short integer alias.
    using atomic_ushort = atomic<u16>;
    /// @brief Atomic 32-bit signed integer alias.
    using atomic_i32 = atomic<i32>;
    /// @brief Atomic 32-bit unsigned integer alias.
    using atomic_u32 = atomic<u32>;
    /// @brief Atomic long integer alias.
    using atomic_long = atomic<long>;
    /// @brief Atomic unsigned long integer alias.
    using atomic_ulong = atomic<ulong>;
    /// @brief Atomic 64-bit signed integer alias.
    using atomic_i64 = atomic<i64>;
    /// @brief Atomic 64-bit unsigned integer alias.
    using atomic_u64 = atomic<u64>;
    /// @brief Atomic UTF-8 character alias.
    using atomic_char8 = atomic<char8>;
    /// @brief Atomic UTF-16 character alias.
    using atomic_char16 = atomic<char16>;
    /// @brief Atomic UTF-32 character alias.
    using atomic_char32 = atomic<char32>;
    /// @brief Atomic wide character alias.
    using atomic_wchar = atomic<wchar>;
    /// @brief Atomic signed pointer-sized integer alias.
    using atomic_iptr = atomic<iptr>;
    /// @brief Atomic unsigned pointer-sized integer alias.
    using atomic_uptr = atomic<uptr>;
    /// @brief Atomic signed size alias.
    using atomic_isize = atomic<isize>;
    /// @brief Atomic unsigned size alias.
    using atomic_usize = atomic<usize>;
    /// @brief Atomic pointer-difference alias.
    using atomic_pdiff = atomic<pdiff>;
    /// @brief Atomic maximum-width signed integer alias.
    using atomic_imax = atomic<imax>;
    /// @brief Atomic maximum-width unsigned integer alias.
    using atomic_umax = atomic<umax>;

    /// @brief Atomic reference that aliases an externally owned object.
    template <typename T>
    using atomic_ref = atomic_detail::atomic_impl<T, true>;

    /// @brief Performs a sequentially consistent atomic store.
    /// @param obj Atomic object receiving the value.
    /// @param dest Value written to `obj`.
    template <typename T>
    static inline void atomic_store(atomic<T>* obj,
        typename atomic<T>::value_type dest)
    {
        obj->store(dest);
    }

    /// @brief Performs a sequentially consistent volatile atomic store.
    /// @param obj Volatile atomic object receiving the value.
    /// @param dest Value written to `obj`.
    template <typename T>
    static inline void atomic_store(volatile atomic<T>* obj,
        typename atomic<T>::value_type dest)
    {
        obj->store(dest);
    }

    /// @brief Performs an atomic store with an explicit memory order.
    /// @param obj Atomic object receiving the value.
    /// @param dest Value written to `obj`.
    /// @param order Memory order applied to the store.
    template <typename T>
    static inline void atomic_store_explicit(atomic<T>* obj,
        typename atomic<T>::value_type dest, memory_order order)
    {
        obj->store(dest, order);
    }

    /// @brief Performs a volatile atomic store with an explicit memory order.
    /// @param obj Volatile atomic object receiving the value.
    /// @param dest Value written to `obj`.
    /// @param order Memory order applied to the store.
    template <typename T>
    static inline void atomic_store_explicit(volatile atomic<T>* obj,
        typename atomic<T>::value_type dest, memory_order order)
    {
        obj->store(dest, order);
    }

    /// @brief Performs a sequentially consistent atomic load.
    /// @param obj Atomic object read by the operation.
    /// @return The value observed by the load.
    template <typename T>
    static inline T atomic_load(atomic<T>* obj)
    {
        return obj->load();
    }

    /// @brief Performs a sequentially consistent volatile atomic load.
    /// @param obj Volatile atomic object read by the operation.
    /// @return The value observed by the load.
    template <typename T>
    static inline T atomic_load(volatile atomic<T>* obj)
    {
        return obj->load();
    }

    /// @brief Performs an atomic load with an explicit memory order.
    /// @param obj Atomic object read by the operation.
    /// @param order Memory order applied to the load.
    /// @return The value observed by the load.
    template <typename T>
    static inline T atomic_load_explicit(
        atomic<T>* obj, memory_order order)
    {
        return obj->load(order);
    }

    /// @brief Performs a volatile atomic load with an explicit memory order.
    /// @param obj Volatile atomic object read by the operation.
    /// @param order Memory order applied to the load.
    /// @return The value observed by the load.
    template <typename T>
    static inline T atomic_load_explicit(
        volatile atomic<T>* obj, memory_order order)
    {
        return obj->load(order);
    }

    /// @brief Atomically exchanges a value using sequential consistency.
    /// @param obj Atomic object to update.
    /// @param dest Replacement value.
    /// @return The value present before the exchange.
    template <typename T>
    static inline T atomic_exchange(
        atomic<T>* obj, 
        typename atomic<T>::value_type dest)
    {
        return obj->exchange(dest);
    }

    /// @brief Volatile sequentially consistent atomic exchange.
    /// @param obj Volatile atomic object to update.
    /// @param dest Replacement value.
    /// @return The value present before the exchange.
    template <typename T>
    static inline T atomic_exchange(
        volatile atomic<T> *obj, 
        typename atomic<T>::value_type dest)
    {
        return obj->exchange(dest);
    }

    /// @brief Atomically exchanges a value with an explicit memory order.
    /// @param obj Atomic object to update.
    /// @param dest Replacement value.
    /// @param order Memory order applied to the exchange.
    /// @return The value present before the exchange.
    template <typename T>
    static inline T atomic_exchange_explicit(
        atomic<T>* obj,
        typename atomic<T>::value_type dest, 
        memory_order order)
    {
        return obj->exchange(dest, order);
    }

    /// @brief Volatile atomic exchange with an explicit memory order.
    /// @param obj Volatile atomic object to update.
    /// @param dest Replacement value.
    /// @param order Memory order applied to the exchange.
    /// @return The value present before the exchange.
    template <typename T>
    static inline T atomic_exchange_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type dest, 
        memory_order order)
    {
        return obj->exchange(dest, order);
    }

    /// @brief Weak sequentially consistent compare/exchange wrapper.
    /// @param obj Atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_weak(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_weak(*expected, desired);
    }

    /// @brief Volatile weak sequentially consistent compare/exchange wrapper.
    /// @param obj Volatile atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_weak(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_weak(*expected, desired);
    }

    /// @brief Strong sequentially consistent compare/exchange wrapper.
    /// @param obj Atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_strong(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_strong(*expected, desired);
    }

    /// @brief Volatile strong sequentially consistent compare/exchange wrapper.
    /// @param obj Volatile atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_strong(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired)
    {
        return obj->compare_exchange_strong(*expected, desired);
    }

    /// @brief Weak compare/exchange wrapper with explicit success and failure orders.
    /// @param obj Atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @param succ Order used after a successful exchange.
    /// @param fail Order used after a failed comparison.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_weak_explicit(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_weak(*expected, desired, succ, fail);
    }

    /// @brief Volatile weak compare/exchange with explicit memory orders.
    /// @param obj Volatile atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @param succ Order used after a successful exchange.
    /// @param fail Order used after a failed comparison.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_weak_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_weak(*expected, desired, succ, fail);
    }

    /// @brief Strong compare/exchange wrapper with explicit success and failure orders.
    /// @param obj Atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @param succ Order used after a successful exchange.
    /// @param fail Order used after a failed comparison.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_strong_explicit(
        atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_strong(*expected, desired, succ, fail);
    }

    /// @brief Volatile strong compare/exchange with explicit memory orders.
    /// @param obj Volatile atomic object to compare and update.
    /// @param expected Expected value; updated with the observed value on failure.
    /// @param desired Replacement value when the comparison succeeds.
    /// @param succ Order used after a successful exchange.
    /// @param fail Order used after a failed comparison.
    /// @return Whether the replacement occurred.
    template <typename T>
    static inline bool atomic_compare_exchange_strong_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::value_type* expected,
        typename atomic<T>::value_type desired,
        memory_order succ, memory_order fail)
    {
        return obj->compare_exchange_strong(*expected, desired, succ, fail);
    }

    /// @brief Atomically adds an arithmetic or pointer difference.
    /// @param obj Atomic object to update.
    /// @param arg Arithmetic increment or pointer distance.
    /// @return The value present before the addition.
    template <typename T>
    static inline T atomic_fetch_add(
        atomic<T>* obj, 
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_add(arg);
    }

    /// @brief Volatile atomic add wrapper.
    /// @param obj Volatile atomic object to update.
    /// @param arg Arithmetic increment or pointer distance.
    /// @return The value present before the addition.
    template <typename T>
    static inline T atomic_fetch_add(
        volatile atomic<T>* obj, 
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_add(arg);
    }

    /// @brief Atomic add with an explicit memory order.
    /// @param obj Atomic object to update.
    /// @param arg Arithmetic increment or pointer distance.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the addition.
    template <typename T>
    static inline T atomic_fetch_add_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_add(arg, order);
    }

    /// @brief Volatile atomic add with an explicit memory order.
    /// @param obj Volatile atomic object to update.
    /// @param arg Arithmetic increment or pointer distance.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the addition.
    template <typename T>
    static inline T atomic_fetch_add_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_add(arg, order);
    }

    /// @brief Atomically applies bitwise AND and returns the old value.
    /// @param obj Integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_and(
        atomic<T>* obj, 
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_and(arg);
    }

    /// @brief Volatile atomic bitwise AND wrapper.
    /// @param obj Volatile integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_and(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_and(arg);
    }

    /// @brief Atomic bitwise AND with an explicit memory order.
    /// @param obj Integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_and_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_and(arg, order);
    }

    /// @brief Volatile atomic bitwise AND with an explicit memory order.
    /// @param obj Volatile integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_and_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_and(arg, order);
    }

    /// @brief Atomically applies bitwise OR and returns the old value.
    /// @param obj Integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_or(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_or(arg);
    }

    /// @brief Volatile atomic bitwise OR wrapper.
    /// @param obj Volatile integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_or(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_or(arg);
    }

    /// @brief Atomic bitwise OR with an explicit memory order.
    /// @param obj Integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_or_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg,
         memory_order order)
    {
        return obj->fetch_or(arg, order);
    }

    /// @brief Volatile atomic bitwise OR with an explicit memory order.
    /// @param obj Volatile integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_or_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_or(arg, order);
    }

    /// @brief Atomically applies bitwise XOR and returns the old value.
    /// @param obj Integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_xor(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_xor(arg);
    }

    /// @brief Volatile atomic bitwise XOR wrapper.
    /// @param obj Volatile integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_xor(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg)
    {
        return obj->fetch_xor(arg);
    }

    /// @brief Atomic bitwise XOR with an explicit memory order.
    /// @param obj Integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_xor_explicit(
        atomic<T>* obj,
        typename atomic<T>::difference_type arg, 
        memory_order order)
    {
        return obj->fetch_xor(arg, order);
    }

    /// @brief Volatile atomic bitwise XOR with an explicit memory order.
    /// @param obj Volatile integral atomic object to update.
    /// @param arg Mask applied to the value.
    /// @param order Memory order applied to the operation.
    /// @return The value present before the operation.
    template <typename T>
    static inline T atomic_fetch_xor_explicit(
        volatile atomic<T>* obj,
        typename atomic<T>::difference_type arg,
         memory_order order)
    {
        return obj->fetch_xor(arg, order);
    }

    /// @brief Lock-free boolean flag for spin and one-time protocols.
    /// @details The flag has no copy or assignment operations. `test_and_set`
    /// returns the old state while setting the flag, and `clear` resets it.
    struct atomic_flag
    {
        /// @brief Constructs a clear flag.
        constexpr atomic_flag()
            : _flag{}
        {}

        constexpr atomic_flag(const atomic_flag&) = delete;
        constexpr atomic_flag& operator=(const atomic_flag&) = delete;
        constexpr atomic_flag& operator=(const atomic_flag&) volatile = delete;

        /// @brief Clears the flag with the selected memory order.
        /// @param order Ordering applied to the clear.
        inline void clear(memory_order order = memory_order_seq_cst)
        {
            __atomic_clear(&_flag, static_cast<i32>(order));
        }

        /// @brief Volatile flag clear with the selected memory order.
        /// @param order Ordering applied to the clear.
        inline void clear(memory_order order = memory_order_seq_cst) volatile
        {
            __atomic_clear(&_flag, static_cast<i32>(order));
        }

        /// @brief Sets the flag and returns its previous state.
        /// @param order Ordering applied to the test-and-set.
        /// @return Previous flag state.
        inline bool test_and_set(memory_order order = memory_order_seq_cst)
        {
            return __atomic_test_and_set(&_flag, static_cast<i32>(order));
        }

        /// @brief Volatile flag test-and-set with the selected memory order.
        /// @param order Ordering applied to the test-and-set.
        /// @return Previous flag state.
        inline bool test_and_set(memory_order order = memory_order_seq_cst) volatile
        {
            return __atomic_test_and_set(&_flag, static_cast<i32>(order));
        }

        /// @brief Reads the flag with the selected memory order.
        /// @param order Ordering applied to the load.
        /// @return Current flag state.
        inline bool test(memory_order order = memory_order_seq_cst) const
        {
            return __atomic_load_n(&_flag, static_cast<i32>(order));
        }

        /// @brief Volatile flag read with the selected memory order.
        /// @param order Ordering applied to the load.
        /// @return Current flag state.
        inline bool test(memory_order order = memory_order_seq_cst) const volatile
        {
            return __atomic_load_n(&_flag, static_cast<i32>(order));
        }

    private:
        /// @brief Byte storage passed to the compiler atomic builtins.
        uchar _flag;
    };

    /// @brief Sequentially consistent atomic flag test-and-set.
    /// @param flag Flag to set.
    /// @return Previous flag state.
    static inline bool atomic_flag_test_and_set(atomic_flag* flag)
    {
        return flag->test_and_set();
    }

    /// @brief Volatile sequentially consistent atomic flag test-and-set.
    /// @param flag Volatile flag to set.
    /// @return Previous flag state.
    static inline bool atomic_flag_test_and_set(volatile atomic_flag* flag)
    {
        return flag->test_and_set();
    }

    /// @brief Atomic flag test-and-set with an explicit memory order.
    /// @param flag Flag to set.
    /// @param order Ordering applied to the operation.
    /// @return Previous flag state.
    static inline bool atomic_flag_test_and_set_explicit(
        atomic_flag* flag, memory_order order)
    {
        return flag->test_and_set(order);
    }

    /// @brief Volatile atomic flag test-and-set with an explicit memory order.
    /// @param flag Volatile flag to set.
    /// @param order Ordering applied to the operation.
    /// @return Previous flag state.
    static inline bool atomic_flag_test_and_set_explicit(
        volatile atomic_flag* flag, memory_order order)
    {
        return flag->test_and_set(order);
    }

    /// @brief Sequentially consistent atomic flag clear.
    /// @param flag Flag to clear.
    static inline void atomic_flag_clear(atomic_flag* flag)
    {
        flag->clear();
    }

    /// @brief Volatile sequentially consistent atomic flag clear.
    /// @param flag Volatile flag to clear.
    static inline void atomic_flag_clear(volatile atomic_flag* flag)
    {
        flag->clear();
    }

    /// @brief Atomic flag clear with an explicit memory order.
    /// @param flag Flag to clear.
    /// @param order Ordering applied to the operation.
    static inline void atomic_flag_clear_explicit(
        atomic_flag* flag, memory_order order)
    {
        flag->clear(order);
    }

    /// @brief Volatile atomic flag clear with an explicit memory order.
    /// @param flag Volatile flag to clear.
    /// @param order Ordering applied to the operation.
    static inline void atomic_flag_clear_explicit(
        volatile atomic_flag* flag, memory_order order)
    {
        flag->clear(order);
    }

    /// @brief Sequentially consistent atomic flag test.
    /// @param flag Flag to read.
    /// @return Current flag state.
    static inline bool atomic_flag_test(atomic_flag* flag)
    {
        return flag->test();
    }

    /// @brief Volatile sequentially consistent atomic flag test.
    /// @param flag Volatile flag to read.
    /// @return Current flag state.
    static inline bool atomic_flag_test(volatile atomic_flag* flag)
    {
        return flag->test();
    }

    /// @brief Atomic flag test with an explicit memory order.
    /// @param flag Flag to read.
    /// @param order Ordering applied to the operation.
    /// @return Current flag state.
    static inline bool atomic_flag_test_explicit(
        atomic_flag* flag, memory_order order)
    {
        return flag->test(order);
    }

    /// @brief Volatile atomic flag test with an explicit memory order.
    /// @param flag Volatile flag to read.
    /// @param order Ordering applied to the operation.
    /// @return Current flag state.
    static inline bool atomic_flag_test_explicit(
        volatile atomic_flag* flag, memory_order order)
    {
        return flag->test(order);
    }
} // namespace hsd