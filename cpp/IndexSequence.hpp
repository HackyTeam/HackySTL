#pragma once

#include <type_traits>
#include "Types.hpp"

namespace hsd
{   
    template< typename T, T... Ints >
    struct integer_sequence
    {
        static constexpr usize size() noexcept
        {
            return sizeof...(Ints);
        }
    };
    
    template <usize N, usize... Next>
    struct index_sequence_helper : public index_sequence_helper<N - 1, N - 1, Next...> {};
    
    template <usize... Next>
    struct index_sequence_helper<0, Next...>
    { 
        using type = integer_sequence<usize, Next...>;
    };
    
    template <usize... Next>
    using index_sequence = integer_sequence<usize, Next...>;
    
    template <usize N>
    using make_index_sequence = typename index_sequence_helper<N>::type;
    
    template<typename T, T N>
    using make_integer_sequence = make_index_sequence<N>;
    
    template<typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}