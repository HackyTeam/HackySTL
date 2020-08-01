#pragma once

#include <type_traits>

namespace hsd
{
    using size_t = unsigned long int;
    
    template< typename T, T... Ints >
    struct integer_sequence
    {
        static constexpr size_t size() noexcept
        {
            return sizeof...(Ints);
        }
    };
    
    template <size_t N, size_t... Next>
    struct index_sequence_helper : public index_sequence_helper<N - 1, N - 1, Next...> {};
    
    template <size_t... Next>
    struct index_sequence_helper<0, Next...>
    { 
        using type = integer_sequence<size_t, Next...>;
    };
    
    template <size_t... Next>
    using index_sequence = integer_sequence<size_t, Next...>;
    
    template <size_t N>
    using make_index_sequence = typename index_sequence_helper<N>::type;
    
    template<typename T, T N>
    using make_integer_sequence = make_index_sequence<N>;
    
    template<typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}