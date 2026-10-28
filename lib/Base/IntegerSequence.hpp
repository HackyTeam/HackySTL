#pragma once

/// @file IntegerSequence.hpp
/// @brief Compile-time integer and index sequence generators.
/// @details Includes forward, ranged, inverse, and type-pack index sequences
/// for tuple expansion, metaprogramming, and constexpr dispatch.

#include "TypeTraits.hpp"

namespace hsd
{
    ///
    /// @brief Compile-time pack of values of a single integral type.
    ///
    /// @tparam T - Integral type of the stored values.
    /// @tparam Ints - Values carried by this sequence.
    ///
    template < typename T, T... Ints >
    struct integer_sequence
    {
        ///
        /// @brief Gets the number of values carried by this sequence.
        ///
        /// @return `sizeof...(Ints)`.
        ///
        static constexpr usize size() noexcept
        {
            return sizeof...(Ints);
        }
    };

    ///
    /// @brief Recursive builder counting down from `Prev` to `To`, prepending
    /// each visited value to `Next` until `integer_sequence<T, Next...>` results.
    ///
    /// @tparam T - Integral type of the generated sequence.
    /// @tparam To - Exclusive lower bound; recursion stops once `Prev == To`.
    /// @tparam Prev - Next value to prepend before recursing further.
    /// @tparam Next - Values already accumulated, most recently prepended first.
    ///
    template < typename T, T To, T Prev, T... Next >
    struct make_integer_sequence_helper
        : hsd::conditional_t< Prev == To, integer_sequence< T, Next... >, 
        make_integer_sequence_helper< T, To, Prev - 1, Prev - 1, Next... >>
    {};

    /// @brief Compile-time pack of `usize` indices.
    template <usize... Next>
    using index_sequence = integer_sequence<usize, Next...>;

    /// @brief Builds an ascending `index_sequence` covering `[0, Size)`.
    template <usize Size>
    using make_index_sequence = make_integer_sequence_helper<usize, 0, Size>;
    
    /// @brief Builds an ascending `integer_sequence<T, ...>` covering `[0, Size)`.
    template <typename T, T Size>
    using make_integer_sequence = make_integer_sequence_helper<T, 0, Size>;
    
    /// @brief Builds an `index_sequence` with one index per type in `T...`.
    template <typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
    
    namespace ranged
    {
        ///
        /// @brief Builds an ascending `integer_sequence<T, ...>` covering `[From, To)`.
        ///
        /// @tparam T - Integral type of the generated sequence.
        /// @tparam From - Inclusive lower bound; requires `From <= To`.
        /// @tparam To - Exclusive upper bound.
        ///
        template < typename T, T From, T To > requires(From <= To)
        using make_integer_sequence = make_integer_sequence_helper< T, From, To >;

        ///
        /// @brief Builds an ascending `index_sequence` covering `[From, To)`.
        ///
        /// @tparam From - Inclusive lower bound; requires `From <= To`.
        /// @tparam To - Exclusive upper bound.
        ///
        template < usize From, usize To > requires(From <= To)
        using make_index_sequence = make_integer_sequence_helper< usize, From, To >;
    } // namespace range

    namespace inverse
    {
        ///
        /// @brief Recursive builder counting up from `Prev` to `To`, prepending
        /// the value below `Prev` at each step until `integer_sequence<T, Next...>`
        /// results, producing a descending sequence.
        ///
        /// @tparam T - Integral type of the generated sequence.
        /// @tparam To - Exclusive upper bound; recursion stops once `Prev == To`.
        /// @tparam Prev - Current value; `Prev` itself is prepended before recursing.
        /// @tparam Next - Values already accumulated, most recently prepended first.
        ///
        template < typename T, T To, T Prev, T... Next >
        struct make_inverse_sequence_helper
            : hsd::conditional_t< Prev == To, integer_sequence< T, Next... >, 
            make_inverse_sequence_helper< T, To, Prev + 1, Prev, Next... >>
        {};

        /// @brief Builds a descending `index_sequence` covering `Size-1 .. 0`.
        template <usize Size>
        using make_index_sequence = make_inverse_sequence_helper<usize, Size, 0>;

        /// @brief Builds a descending `integer_sequence<T, ...>` covering `Size-1 .. 0`.
        template <typename T, T Size>
        using make_integer_sequence = make_inverse_sequence_helper<T, Size, 0>;

        /// @brief Builds a descending `index_sequence` with one index per type in `T...`.
        template <typename... T>
        using index_sequence_for = inverse::make_index_sequence<sizeof...(T)>;

        namespace ranged
        {
            ///
            /// @brief Builds a descending `integer_sequence<T, ...>` covering the
            /// half-open range `[To, From)`, i.e. `From-1` down to `To`.
            ///
            /// @tparam T - Integral type of the generated sequence.
            /// @tparam From - Exclusive upper bound; requires `From >= To`.
            /// @tparam To - Inclusive lower bound.
            ///
            template < typename T, T From, T To > requires(From >= To)
            using make_integer_sequence = make_inverse_sequence_helper< T, From, To >;

            ///
            /// @brief Builds a descending `index_sequence` covering the half-open
            /// range `[To, From)`, i.e. `From-1` down to `To`.
            ///
            /// @tparam From - Exclusive upper bound; requires `From >= To`.
            /// @tparam To - Inclusive lower bound.
            ///
            template < usize From, usize To > requires(From >= To)
            using make_index_sequence = make_inverse_sequence_helper< usize, From, To >;
        } // namespace range
        
    } // namespace inverse
}
