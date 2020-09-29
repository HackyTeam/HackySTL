#pragma once

#include "TypeTraits.hpp"
#include "Types.hpp"

namespace hsd
{
    template < typename T, T... Ints >
    struct integer_sequence
    {
        static constexpr usize size() noexcept
        {
            return sizeof...(Ints);
        }
    };

    template <typename T, T... Next>
    struct make_integer_sequence_helper_alternative
    {
        using type = integer_sequence<T, Next...>;
    };

    template < typename T, T Prev, T... Next >
    struct make_integer_sequence_helper
        : public hsd::conditional_t<Prev == 0, 
        make_integer_sequence_helper_alternative< T, Next... >, 
        make_integer_sequence_helper< T, Prev - 1, Prev - 1, Next... >>
    {};

    template <usize... Next>
    using index_sequence = integer_sequence<usize, Next...>;
    
    template <usize N>
    using make_index_sequence = typename make_integer_sequence_helper<usize, N>::type;
    
    template <typename T, T Size> requires(Size >= 0)
    using make_integer_sequence = typename make_integer_sequence_helper<T, Size>::type;
    
    template <typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}