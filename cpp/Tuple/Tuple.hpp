#include <stdexcept>

template < typename... T>
class tuple_helper {};

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
    
    template<size_t N>
    constexpr auto get() 
    {
        return get_helper<N, tuple_helper<T, Rest...>>::get(*this);
    }
};

template < typename... T>
using tuple = tuple_helper<T...>;

namespace TupTest
{
    static void Test()
    {
        tuple<int, char, const char*> t(500, 'a', "ABC");
        printf("%d\n", t.get<0>());
    }
}