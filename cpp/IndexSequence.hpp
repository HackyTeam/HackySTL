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

    template < typename T, T Prev, T... Next >
    struct make_integer_sequence_helper
        : hsd::conditional_t<Prev == 0, integer_sequence< T, Next... >, 
        make_integer_sequence_helper< T, Prev - 1, Prev - 1, Next... >>
    {};

    template <usize... Next>
    using index_sequence = integer_sequence<usize, Next...>;
    
    template <usize N>
    using make_index_sequence = make_integer_sequence_helper<usize, N>;
    
    template <typename T, T Size> requires(Size >= 0)
    using make_integer_sequence = make_integer_sequence_helper<T, Size>;
    
    template <typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}