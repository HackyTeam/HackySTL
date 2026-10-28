#pragma once

/// @file Pair.hpp
/// @brief Project pair type with deduction and tuple-like access.
/// @details Stores two heterogeneous values, supports forwarding construction,
/// comparisons, structured bindings, and compile-time element access.

#include "../Base/Utility.hpp"
#include "../Base/Concepts.hpp"

namespace hsd
{
    /// @brief Tests whether both pair element types support boolean equality.
    template <typename T, typename U>
    concept IsComparablePair = requires(const T& lhs, const U& rhs)
    {
        { lhs == lhs } -> IsSame<bool>;
        { rhs == rhs } -> IsSame<bool>;
    };

    /// @brief Two-value aggregate with tuple-like indexed access.
    template <typename T1, typename T2>
    struct pair
    {
        /// @brief First stored value.
        T1 first;
        /// @brief Second stored value.
        T2 second;

        constexpr ~pair() = default;
        constexpr pair() = default;

        /// @brief Constructs both elements from forwarded values.
        /// @param first_val Initial value for `first`.
        /// @param second_val Initial value for `second`.
        template <typename _T1, typename _T2>
        constexpr pair(_T1&& first_val, _T2&& second_val)
            : first{hsd::forward<_T1>(first_val)}, second{hsd::forward<_T2>(second_val)}
        {}

        /// @brief Copies both elements from `other`.
        /// @param other Pair whose values are copied.
        constexpr pair(const pair& other)
            : first{other.first}, second{other.second}
        {}
        
        /// @brief Moves both elements from `other`.
        /// @param other Pair whose values are moved.
        constexpr pair(pair&& other)
            : first{hsd::move(other.first)}, second{hsd::move(other.second)}
        {}

        /// @brief Copy-assigns both elements.
        /// @param rhs Pair whose values are copied.
        /// @return `*this` after assignment.
        constexpr pair& operator=(const pair& rhs)
        {
            first = rhs.first;
            second = rhs.second;
            return *this;
        }
        
        /// @brief Move-assigns both elements.
        /// @param rhs Pair whose values are moved.
        /// @return `*this` after assignment.
        constexpr pair& operator=(pair&& rhs)
        {
            hsd::swap(first, rhs.first);
            hsd::swap(second, rhs.second);
            return *this;
        }

        /// @brief Compares both elements for equality.
        /// @param rhs Pair compared with this pair.
        /// @return True when both elements compare equal.
        constexpr bool operator==(const pair& rhs) const
        requires (IsComparablePair<T1, T2>)
        {
            return first == rhs.first && second == rhs.second;
        }

        /// @brief Compares both elements for inequality.
        /// @param rhs Pair compared with this pair.
        /// @return True when at least one element differs.
        constexpr bool operator!=(const pair& rhs) const
        requires (IsComparablePair<T1, T2>)
        {
            return !(*this == rhs);
        }
    };

    /// @brief Deduction guide for two const lvalue values.
    template <typename T1, typename T2> pair(const T1&, const T2&) 
        -> pair<decay_t<T1>, decay_t<T2>>;
    /// @brief Deduction guide for a forwarded first value and const second value.
    template <typename T1, typename T2> requires (!is_const<T1>::value)
        pair(T1&&, const T2&) -> pair<decay_t<T1>, decay_t<T2>>;
    /// @brief Deduction guide preserving a const first value.
    template <typename T1, typename T2> requires (is_const<T1>::value)
        pair(T1&&, const T2&) -> pair<const decay_t<T1>, decay_t<T2>>;
    /// @brief Deduction guide for a const first value and forwarded second value.
    template <typename T1, typename T2> requires (!is_const<T2>::value)
        pair(const T1&, T2&&) -> pair<decay_t<T1>, decay_t<T2>>;
    /// @brief Deduction guide preserving a const second value.
    template <typename T1, typename T2> requires (is_const<T2>::value)
        pair(const T1&, T2&&) -> pair<decay_t<T1>, const decay_t<T2>>;

    /// @brief Deduction guide for two non-const forwarded values.
    template <typename T1, typename T2> 
    requires (!is_const<T1>::value && !is_const<T2>::value)
        pair(T1&&, T2&&) -> pair<decay_t<T1>, decay_t<T2>>;

    /// @brief Deduction guide preserving a const first forwarded value.
    template <typename T1, typename T2> 
    requires (is_const<T1>::value && !is_const<T2>::value)
        pair(T1&&, T2&&) -> pair<const decay_t<T1>, decay_t<T2>>;

    /// @brief Deduction guide preserving a const second forwarded value.
    template <typename T1, typename T2> 
    requires (!is_const<T1>::value && is_const<T2>::value)
        pair(T1&&, T2&&)-> pair<decay_t<T1>, const decay_t<T2>>;

    /// @brief Deduction guide preserving both const forwarded values.
    template <typename T1, typename T2>
    requires (is_const<T1>::value && is_const<T2>::value)
        pair(T1&&, T2&&) -> pair<const decay_t<T1>, const decay_t<T2>>;

    /// @brief Constructs a pair by forwarding both values.
    /// @param first First value forwarded into the pair.
    /// @param second Second value forwarded into the pair.
    /// @return Pair containing the forwarded values.
    template <typename T1, typename T2>
    static constexpr auto make_pair(T1&& first, T2&& second)
    {
        return pair{hsd::forward<T1>(first), hsd::forward<T2>(second)};
    }
} // namespace hsd