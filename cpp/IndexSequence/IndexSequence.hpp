#pragma once

#include <type_traits>
#include <iostream>

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

namespace ISeqTest
{
    static void print(size_t S)
    {
        std::cout << S << ' ';
    }

    template<typename T, T... ints>
    static void print_sequence(integer_sequence<T, ints...> int_seq)
    {
        std::cout << "The sequence of size " << int_seq.size() << ": ";
        (print(ints), ...);
        std::cout << '\n';
    }

    static void Test()
    {
        print_sequence(integer_sequence<unsigned, 9, 2, 5, 1, 9, 1, 6>{});
        print_sequence(make_integer_sequence<int, 20>{});
        print_sequence(make_index_sequence<10>{});
        print_sequence(index_sequence_for<float, std::iostream, char>{});
    }
}