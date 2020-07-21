#pragma once

#include "../IndexSequence/IndexSequence.hpp"
#include <utility>

using size_t = unsigned long int;

template< typename T1, typename T2 >
struct pair
{
    T1 fist;
    T2 second;
};

template < typename... T >
class tuple
{
public:
    static constexpr size_t size()
    {
        return sizeof...(T);
    }
};

template< typename... T > tuple<T...> make_tuple(T&&... args);

template< typename T, typename... Rest >
class tuple<T, Rest...>
{   
public:   
    T _first;
    tuple<Rest...> _rest;        

private:
    template< size_t N, typename U >
    struct get_helper {};

    template< typename U, typename... Args >
    struct get_helper<0, tuple<U, Args...>>
    {
        static constexpr U get(const tuple<U, Args...>& data)
        {
            return data._first;
        }
    };

    template< size_t N, typename U, typename... Args >
    struct get_helper<N, tuple<U, Args...>>
    {
        static constexpr auto get(const tuple<U, Args...>& data)
        {
            return get_helper<N - 1, tuple<Args...>>::get(data._rest);
        }
    };
public:
    constexpr tuple(const T& first, const Rest& ... rest) 
        : _first{first}, _rest{rest...} {}

    constexpr tuple(T&& first, Rest&& ... rest) 
        : _first{std::move(first)}, _rest{std::forward<Rest>(rest)...}
    {}

    constexpr tuple(const tuple& other)
        : _first{other._first}, _rest{other._rest}
    {}

    constexpr tuple(tuple&& other)
    {
        _first = std::move(other._first);
        _rest = std::move(other._rest);
    }

    template< typename... Args >
    constexpr auto operator+(const tuple<Args...>& rhs)
    {
        auto _add = [&]< size_t... Ints1, size_t... Ints2>(
            index_sequence<Ints1...> int_seq1, 
            index_sequence<Ints2...> int_seq2
        ) -> tuple<T, Rest..., Args...>
        {
            return make_tuple<T, Rest..., Args...>
            (
                (this->template get<Ints1>(), ...), 
                (rhs.template get<Ints2>(), ...)
            );
        };

        return _add(index_sequence_for<T , Rest...>{}, index_sequence_for<Args...>{});
    }

    template<size_t N>
    constexpr auto get() 
    {
        return get_helper<N, tuple<T, Rest...>>::get(*this);
    }

    template<size_t N>
    constexpr auto get() const
    {
        return get_helper<N, tuple<T, Rest...>>::get(*this);
    }

    static constexpr size_t size()
    {
        return 1 + sizeof...(Rest);
    }
};

template<class... UTypes> tuple(UTypes...) -> tuple<UTypes...>;
template<class T1, class T2> tuple(pair<T1, T2>) -> tuple<T1, T2>;

template< typename... T >
tuple<T...> make_tuple(T&&... args)
{
    return {std::forward<T>(args)...};
}

template< typename... T >
tuple<T...> tie(T&... args)
{
    return {args...};
}

namespace TupTest
{
    static void Test()
    {
        tuple t = {500, 'a', "more professional print"};
        //auto t2 = t + make_tuple(1, -1, 'c');
        printf("%s\n%zu\n", t.get<2>(), t.size());
    }
}