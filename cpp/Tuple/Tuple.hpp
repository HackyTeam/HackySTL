#pragma once

#include <stdio.h>
#include <utility>

using size_t = unsigned long int;

template< typename T1, typename T2 >
struct pair
{
    T1 fist;
    T2 second;
};


template < typename... T >
class tuple_helper
{
public:
    static constexpr size_t size()
    {
        return sizeof...(T);
    }
};

template< typename T, typename... Rest >
class tuple_helper<T, Rest...>
{   
public:   
    T _first;
    tuple_helper<Rest...> _rest;        

private:
    template< size_t N, typename U >
    struct get_helper {};

    template< typename U, typename... Args >
    struct get_helper<0, tuple_helper<U, Args...>>
    {
        static constexpr U get(tuple_helper<U, Args...>& data)
        {
            return data._first;
        }
    };

    template< size_t N, typename U, typename... Args >
    struct get_helper<N, tuple_helper<U, Args...>>
    {
        static constexpr auto get(tuple_helper<U, Args...>& data)
        {
            return get_helper<N - 1, tuple_helper<Args...>>::get(data._rest);
        }
    };
public:
    constexpr tuple_helper(const T& first, const Rest& ... rest) 
        : _first(first), _rest(rest...) {}

    constexpr tuple_helper(const tuple_helper& other)
        : _first{other._first}, _rest{other._rest}
    {}

    constexpr tuple_helper(tuple_helper&& other)
    {
        _first = std::move(other._first);
        _rest = std::move(other._rest);
    }
    
    template<size_t N>
    constexpr auto get() 
    {
        return get_helper<N, tuple_helper<T, Rest...>>::get(*this);
    }

    static constexpr size_t size()
    {
        return 1 + sizeof...(Rest);
    }
};

template<class... UTypes> tuple_helper(UTypes...) -> tuple_helper<UTypes...>;
template<class T1, class T2> tuple_helper(pair<T1, T2>) -> tuple_helper<T1, T2>;

template < typename... T>
using tuple = tuple_helper<T...>;

template< typename... T >
tuple<T...> make_tuple(const T&... args)
{
    return {args...};
}

namespace TupTest
{
    static void Test()
    {
        tuple t = {500, 'a', "more professional print"};
        printf("%s\n%d\n", t.get<2>(), t.size());
    }
}