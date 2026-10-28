#pragma once

/// @file Utility.hpp
/// @brief Core forwarding, lifetime, memory, and generic utility operations.
/// @details Provides move/forward, construct_at, aligned raw allocation,
/// address, swap, copy, search, and constant-evaluation helpers used broadly.

#include "TypeTraits.hpp"

// Required to unlock std::construct_at which
// allows us to in-place construct at compile time
#if __has_include(<bits/stl_construct.h>)
#include <bits/stl_construct.h>
#elif __has_include(<xutility>)
#include <xutility>
#endif

#include <new>

#if !defined(HSD_COMPILER_GCC) && !defined(HSD_COMPILER_CLANG)
#include <string.h>
#endif

#ifdef HSD_COMPILER_MSVC
#undef min
#undef max
#endif

namespace hsd
{
    ///
    /// @brief Casts a value to an rvalue reference, enabling move construction.
    ///
    /// @tparam T - Deduced type of the argument.
    /// @param val - Value to be treated as movable.
    /// @return An rvalue reference to `val`.
    ///
    template <typename T>
    static constexpr remove_reference_t<T>&& move(T&& val)
    {
        return static_cast<remove_reference_t<T>&&>(val);
    }

    ///
    /// @brief Casts a value to an rvalue reference and reads it by value.
    ///
    /// @tparam T - Deduced type of the argument.
    /// @param val - Value to be moved out of.
    /// @return A prvalue copy constructed from `val` as an rvalue.
    ///
    template <typename T>
    static constexpr remove_reference_t<T> release(T&& val)
    {
        return static_cast<remove_reference_t<T>&&>(val);
    }

    ///
    /// @brief Forwards an lvalue as the value category requested by `T`.
    ///
    /// @tparam T - Deduced parameter type of the forwarding function.
    /// @param val - Lvalue reference bound to the forwarding parameter.
    /// @return `val` cast to `T&&`.
    ///
    template <typename T>
    static constexpr T&& forward(remove_reference_t<T>& val)
    {
        return static_cast<T&&>(val);
    }

    ///
    /// @brief Forwards an rvalue as the value category requested by `T`.
    ///
    /// @tparam T - Deduced parameter type of the forwarding function.
    /// @param val - Rvalue reference bound to the forwarding parameter.
    /// @return `val` cast to `T&&`.
    ///
    template <typename T>
    static constexpr T&& forward(remove_reference_t<T>&& val)
    {
        return static_cast<T&&>(val);
    }

    ///
    /// @brief Replaces `target` with `new_val` and returns the previous value.
    ///
    /// @tparam T - Type of the value being replaced.
    /// @tparam U - Type of the replacement value, convertible to `T`.
    /// @param target - Object whose value is replaced.
    /// @param new_val - Value moved or converted into `target`.
    /// @return The value `target` held before the call.
    ///
    template <typename T, typename U = T>
    static constexpr T exchange(T& target, U&& new_val) noexcept
    {
        T tmp = hsd::move(target);
        target = hsd::forward<U>(new_val);
        return tmp;
    }

    ///
    /// @brief Move-assigns each element of `[first, last)` into `dest`.
    ///
    /// @tparam InIt - Input iterator type.
    /// @tparam OutIt - Output iterator type.
    /// @param first - Start of the source range.
    /// @param last - End of the source range, exclusive.
    /// @param dest - Start of the destination range.
    /// @return An iterator one past the last written element.
    ///
    template <typename InIt, typename OutIt>
    static constexpr OutIt move(InIt first, InIt last, OutIt dest)
    {
        while (first != last) 
            *dest++ = hsd::move(*first++);

        return dest;
    }

    ///
    /// @brief Tests whether the calling context is evaluated at compile time.
    ///
    /// @return `true` during constant evaluation, `false` at runtime.
    ///
    [[nodiscard]] static constexpr bool is_constant_evaluated()
    {
        return __builtin_is_constant_evaluated();
    }

    ///
    /// @brief Reinterprets the bit representation of a value as another type.
    ///
    /// @tparam To - Destination type; must be the same size as the source.
    /// @param from - Value whose bit pattern is reinterpreted.
    /// @return A `To` value with the same bit pattern as `from`.
    ///
    template <typename To>
    [[nodiscard]] static constexpr auto bit_cast(auto from)
    {
        return __builtin_bit_cast(To, from);
    }

    ///
    /// @brief Obtains a pointer to the object that now occupies storage at `ptr`.
    ///
    /// @details Wraps `__builtin_launder` on GCC and Clang and `std::launder`
    /// elsewhere. Use it after an object of the same type has been placed over
    /// another one in the same storage, so the compiler does not reuse a pointer
    /// to the replaced object.
    ///
    /// @tparam T - Object type the pointer refers to.
    /// @param ptr - Pointer to the storage holding the new object.
    /// @return A pointer that refers to the object now stored at `ptr`.
    ///
    template <typename T>
    [[nodiscard]] static constexpr T* launder(T* ptr) noexcept
    {
        #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
        return __builtin_launder(ptr);
        #else
        return std::launder(ptr);
        #endif
    }

    ///
    /// @brief Counts the zero bits below the least significant set bit.
    ///
    /// @details Compiles to a single count-trailing-zeros instruction on GCC
    /// and Clang, including during constant evaluation. Other compilers fall
    /// back to a binary search over halving bit widths, which needs no tables
    /// or intrinsics.
    ///
    /// @tparam T - Unsigned integer type.
    /// @param val - Value whose trailing zero bits are counted.
    /// @return Number of trailing zero bits, or the bit width of `T` when
    /// `val` is zero.
    ///
    template <typename T> requires (is_unsigned<T>::value)
    [[nodiscard]] static constexpr i32 countr_zero(T val) noexcept
    {
        constexpr i32 _digits = static_cast<i32>(sizeof(T) * 8);

        if (val == 0)
        {
            return _digits;
        }

        #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
        if constexpr (sizeof(T) <= sizeof(u32))
        {
            return __builtin_ctz(static_cast<u32>(val));
        }
        else if constexpr (sizeof(T) <= sizeof(ulong))
        {
            return __builtin_ctzl(static_cast<ulong>(val));
        }
        else if constexpr (sizeof(T) <= sizeof(u64))
        {
            return __builtin_ctzll(static_cast<u64>(val));
        }
        #endif

        i32 _count = 0;

        for (i32 _width = _digits / 2; _width > 0; _width /= 2)
        {
            if ((val & static_cast<T>((T{1} << _width) - 1)) == 0)
            {
                _count += _width;
                val = static_cast<T>(val >> _width);
            }
        }

        return _count;
    }

    ///
    /// @brief Counts the zero bits above the most significant set bit.
    ///
    /// @details Compiles to a single count-leading-zeros instruction on GCC
    /// and Clang, including during constant evaluation. Other compilers fall
    /// back to a binary search over halving bit widths, which needs no tables
    /// or intrinsics.
    ///
    /// @tparam T - Unsigned integer type.
    /// @param val - Value whose leading zero bits are counted.
    /// @return Number of leading zero bits, or the bit width of `T` when
    /// `val` is zero.
    ///
    template <typename T> requires (is_unsigned<T>::value)
    [[nodiscard]] static constexpr i32 countl_zero(T val) noexcept
    {
        constexpr i32 _digits = static_cast<i32>(sizeof(T) * 8);

        if (val == 0)
        {
            return _digits;
        }

        #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
        // The builtins count within their own width, so the bits above `T`
        // are subtracted from the result.
        if constexpr (sizeof(T) <= sizeof(u32))
        {
            return __builtin_clz(static_cast<u32>(val)) - (32 - _digits);
        }
        else if constexpr (sizeof(T) <= sizeof(ulong))
        {
            return __builtin_clzl(static_cast<ulong>(val))
                - (static_cast<i32>(sizeof(ulong) * 8) - _digits);
        }
        else if constexpr (sizeof(T) <= sizeof(u64))
        {
            return __builtin_clzll(static_cast<u64>(val)) - (64 - _digits);
        }
        #endif

        i32 _count = 0;

        for (i32 _width = _digits / 2; _width > 0; _width /= 2)
        {
            if ((val >> (_digits - _width)) == 0)
            {
                _count += _width;
                val = static_cast<T>(val << _width);
            }
        }

        return _count;
    }

    /// @brief Rounds `value` up to the next power of two.
    /// @details Ring positions are wrapped with a bit mask instead of a
    /// division, which requires the capacity to be a power of two. If
    /// rounding up would overflow, `value` is returned unchanged and the
    /// allocation that follows reports the failure.
    /// @param value Minimum capacity needed.
    /// @return Smallest power of two not below `value` (at least one).
    static constexpr usize pow2_ceil(usize value)
    {
        value |= 2;
        constexpr usize _bit_length = 8 * sizeof(value);
        return static_cast<usize>(1) << (_bit_length - countl_zero(value - 1));
    }

    ///
    /// @brief Assigns a compile-time-only constructed value to `dest`.
    ///
    /// @tparam T - Type of the destination object.
    /// @tparam Args - Constructor argument types.
    /// @param dest - Object receiving the constructed value.
    /// @param args - Arguments forwarded to `T`'s constructor.
    ///
    template <typename T, typename... Args>
    static consteval void literal_construct(T& dest, Args&&... args)
    {
        dest = T{hsd::forward<Args>(args)...};
    }

    /// @brief Deleted overload that rejects binding `as_const` to an rvalue.
    template <typename T>
    static constexpr void as_const(T&&) = delete;
    
    ///
    /// @brief Gets a const-qualified reference to an lvalue.
    ///
    /// @tparam T - Deduced type of the argument.
    /// @param val - Lvalue to view as const.
    /// @return A `const T&` referring to `val`.
    ///
    template <typename T>
    static constexpr const T& as_const(T& val)
    {
        return val;
    }
    
    ///
    /// @brief Gets a const-qualified view of a pointer's pointee.
    ///
    /// @tparam T - Deduced pointee type.
    /// @param val - Pointer whose pointee is viewed as const.
    /// @return A `const T*` with the same address as `val`.
    ///
    template <typename T>
    static constexpr const T* as_const(T* val)
    {
        return val;
    }
    
    ///
    /// @brief const-qualified overload of `as_const(T&)`.
    ///
    /// @tparam T - Deduced type of the argument.
    /// @param val - Value already const-qualified.
    /// @return A `const T&` referring to `val`.
    ///
    template <typename T>
    static constexpr const T& as_const(const T& val)
    {
        return val;
    }
    
    ///
    /// @brief const-qualified overload of `as_const(T*)`.
    ///
    /// @tparam T - Deduced pointee type.
    /// @param val - Pointer to a const pointee.
    /// @return A `const T*` with the same address as `val`.
    ///
    template <typename T>
    static constexpr const T* as_const(const T* val)
    {
        return val;
    }

    ///
    /// @brief Gets the address of a const object, bypassing `operator&`.
    ///
    /// @tparam Type - Deduced object type.
    /// @param value - Object whose address is requested.
    /// @return The address of `value`.
    ///
    template <typename Type>
    static constexpr const Type* addressof(const Type& value)
    {
        return __builtin_addressof(value);
    }

    ///
    /// @brief Gets the address of an object, bypassing `operator&`.
    ///
    /// @tparam Type - Deduced object type.
    /// @param value - Object whose address is requested.
    /// @return The address of `value`.
    ///
    template <typename Type>
    static constexpr Type* addressof(Type& value)
    {
        return __builtin_addressof(value);
    }

    ///
    /// @brief Exchanges the values of two objects of the same type.
    ///
    /// @tparam T1 - Type of both operands.
    /// @param first - First object.
    /// @param second - Second object.
    ///
    template <typename T1>
    static constexpr void swap(T1& first, T1& second) noexcept
    {
        auto _tmp = hsd::move(first);
        first = hsd::move(second);
        second = hsd::move(_tmp);
    }

    ///
    /// @brief Exchanges the values of two objects of possibly differing types.
    ///
    /// @param first - First object.
    /// @param second - Second object.
    ///
    static constexpr void swap(auto& first, auto& second) noexcept
    {
        auto _tmp = hsd::move(first);
        first = hsd::move(second);
        second = hsd::move(_tmp);
    }
    
    ///
    /// @brief Copy-assigns each element of `[first, last)` into `dest`.
    ///
    /// @tparam InIt - Input iterator type.
    /// @tparam OutIt - Output iterator type.
    /// @param first - Start of the source range.
    /// @param last - End of the source range, exclusive.
    /// @param dest - Start of the destination range.
    /// @return An iterator one past the last written element.
    ///
    template <typename InIt, typename OutIt>
    static constexpr OutIt copy(InIt first, InIt last, OutIt dest)
    {
        while (first != last)
        { 
            *dest++ = *first++;
        }

        return dest;
    }

    ///
    /// @brief Copies exactly `n` elements starting at `first` into `dest`.
    ///
    /// @tparam InIt - Input iterator type.
    /// @tparam OutIt - Output iterator type.
    /// @param first - Start of the source range.
    /// @param n - Number of elements to copy.
    /// @param dest - Start of the destination range.
    /// @return An iterator one past the last written element.
    ///
    template <typename InIt, typename OutIt>
    static constexpr OutIt copy_n(InIt first, usize n, OutIt dest)
    {
        using value_type = remove_reference_t<decltype(*dest)>;

        for (usize _index = 0; _index != n; ++_index)
        {
            *dest++ = static_cast<value_type>(*first++);
        }
    
        return dest;
    }

    ///
    /// @brief Finds the first element in `[first, last)` satisfying `pred`.
    ///
    /// @tparam InIt - Input iterator type.
    /// @tparam Pred - Predicate invocable on a dereferenced iterator.
    /// @param first - Start of the range to search.
    /// @param last - End of the range, exclusive.
    /// @param pred - Predicate returning whether an element matches.
    /// @return An iterator to the first match, or `last` when none is found.
    ///
    template <typename InIt, typename Pred>
    static constexpr auto find(InIt first, InIt last, Pred&& pred)
    {
        for (; first != last; ++first)
        {
            if (pred(*first)) return first;
        }

        return last;
    }

    ///
    /// @brief Assigns `value` to every element of `[first, last)`.
    ///
    /// @tparam InIt - Forward iterator type.
    /// @tparam ValueType - Type assigned to each element.
    /// @param first - Start of the range to fill.
    /// @param last - End of the range, exclusive.
    /// @param value - Value assigned to each element.
    ///
    template <typename InIt, typename ValueType>
    static constexpr void set(InIt first, InIt last, ValueType value)
    {
        for (; first != last; *first++ = value)
            ;
    }

    ///
    /// @brief Copies elements of `[first, last)` satisfying `pred` into `dest`.
    ///
    /// @tparam InIt - Input iterator type.
    /// @tparam OutIt - Output iterator type.
    /// @tparam Pred - Predicate invocable on a dereferenced iterator.
    /// @param first - Start of the source range.
    /// @param last - End of the source range, exclusive.
    /// @param dest - Start of the destination range.
    /// @param pred - Predicate selecting which elements are copied.
    /// @return An iterator one past the last written element.
    ///
    template < typename InIt, typename OutIt, typename Pred >
    static constexpr OutIt copy_if(InIt first, InIt last, OutIt dest, Pred pred)
    {
        while (first != last) 
        {
            if (pred(*first))
            {
                *dest++ = *first;
            }
                
            ++first;
        }
        return dest;
    }

    ///
    /// @brief Gets a pointer to the first element of a fixed-size array.
    ///
    /// @tparam Elem - Array element type.
    /// @tparam Count - Number of elements in the array.
    /// @param arr - Array to view.
    /// @return A pointer to the first element.
    ///
    template <typename Elem, usize Count>
    static constexpr Elem* begin(Elem (&arr)[Count])
    {
        return static_cast<Elem*>(arr);
    }

    ///
    /// @brief Gets a pointer one past the last element of a fixed-size array.
    ///
    /// @tparam Elem - Array element type.
    /// @tparam Count - Number of elements in the array.
    /// @param arr - Array to view.
    /// @return A pointer one past the last element.
    ///
    template <typename Elem, usize Count>
    static constexpr Elem* end(Elem (&arr)[Count])
    {
        return static_cast<Elem*>(arr) + Count;
    }

    ///
    /// @brief const-qualified overload of `begin(Elem (&)[Count])`.
    ///
    /// @tparam Elem - Array element type.
    /// @tparam Count - Number of elements in the array.
    /// @param arr - Array to view.
    /// @return A pointer to the first element.
    ///
    template <typename Elem, usize Count>
    static constexpr const Elem* begin(const Elem (&arr)[Count])
    {
        return static_cast<const Elem*>(arr);
    }


    ///
    /// @brief const-qualified overload of `end(Elem (&)[Count])`.
    ///
    /// @tparam Elem - Array element type.
    /// @tparam Count - Number of elements in the array.
    /// @param arr - Array to view.
    /// @return A pointer one past the last element.
    ///
    template <typename Elem, usize Count>
    static constexpr const Elem* end(const Elem (&arr)[Count])
    {
        return static_cast<const Elem*>(arr) + Count;
    }

    ///
    /// @brief Constructs an object of type `T` in-place at `ptr`.
    ///
    /// @tparam T - Type of the object to construct.
    /// @tparam Args - Constructor argument types.
    /// @param ptr - Uninitialized storage where the object is constructed.
    /// @param args - Arguments forwarded to `T`'s constructor.
    /// @return `ptr`, cast to the constructed object's pointer type.
    ///
    template<typename T, typename... Args>
    static constexpr auto construct_at(T* ptr, Args&&... args)
    {
        return std::construct_at(ptr, hsd::forward<Args>(args)...);
    }

    ///
    /// @brief Copies `n` trivially copyable objects from `first` into `dest`.
    ///
    /// @details Copies the object representation with `__builtin_memcpy` on
    /// GCC and Clang, and with `memcpy` elsewhere. Constant evaluation
    /// constructs each element in place instead, because a byte copy into
    /// uninitialized storage is not a constant expression. An empty range
    /// returns immediately, so `first` and `dest` may be null when `n` is zero.
    ///
    /// @tparam T - Trivially copyable element type.
    /// @param first - Start of the source range.
    /// @param n - Number of elements to copy.
    /// @param dest - Uninitialized storage with room for `n` elements.
    ///
    template <typename T>
    static constexpr void copy_trivial_n(const T* first, usize n, T* dest)
    {
        if (n == 0)
        {
            return;
        }

        if (is_constant_evaluated())
        {
            for (usize _index = 0; _index < n; ++_index)
            {
                hsd::construct_at(dest + _index, first[_index]);
            }

            return;
        }

        #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
        __builtin_memcpy(
            static_cast<void*>(dest), static_cast<const void*>(first),
            n * sizeof(T)
        );
        #else
        memcpy(
            static_cast<void*>(dest), static_cast<const void*>(first),
            n * sizeof(T)
        );
        #endif
    }

    ///
    /// @brief Allocates raw storage with the requested alignment when supported.
    ///
    /// @param size - Number of bytes to allocate.
    /// @param alignment - Required allocation alignment.
    /// @return The allocated address, or `nullptr` on failure.
    ///
    static inline void* new_allocate(
        usize size, [[maybe_unused]] usize alignment)
    {
        #ifdef __cpp_aligned_new
        // Plain operator new already guarantees the default alignment, and
        // skipping the aligned overload avoids its extra allocation layer.
        // `delete_deallocate` applies the same test so the pair always matches.
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
        {
            return ::operator new(
                size, static_cast<std::align_val_t>(alignment), std::nothrow
            );
        }
        #endif

        return ::operator new(size, std::nothrow);
    }

    ///
    /// @brief Releases storage returned by `new_allocate`.
    ///
    /// @param ptr - Address returned by `new_allocate`.
    /// @param size - Byte count passed to `new_allocate`.
    /// @param alignment - Alignment passed to `new_allocate`.
    ///
    static inline void delete_deallocate(
        void* ptr, [[maybe_unused]] usize size,
        [[maybe_unused]] usize alignment)
    {
        #ifdef __cpp_aligned_new
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
        {
            #ifdef __cpp_sized_deallocation
            ::operator delete(
                ptr, size, static_cast<std::align_val_t>(alignment)
            );
            #else
            ::operator delete(ptr, static_cast<std::align_val_t>(alignment));
            #endif
            return;
        }
        #endif

        #ifdef __cpp_sized_deallocation
        ::operator delete(ptr, size);
        #else
        ::operator delete(ptr);
        #endif
    }

    ///
    /// @brief Invokes `func` for each element of a const range.
    ///
    /// @param arr - Range whose elements are visited; must support range-for.
    /// @param func - Callable invoked with a `const` reference to each element.
    ///
    static constexpr void for_each(const auto& arr, auto&& func)
    {
        for (const auto& _iter : arr)
        {
            func(_iter);
        }
    }

    ///
    /// @brief Invokes `func` for each element of a mutable range.
    ///
    /// @param arr - Range whose elements are visited; must support range-for.
    /// @param func - Callable invoked with a mutable reference to each element.
    ///
    static constexpr void for_each(auto& arr, auto&& func)
    {
        for (auto& _iter : arr)
        {
            func(_iter);
        }
    }
} // namespace hsd
