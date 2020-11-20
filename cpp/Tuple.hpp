#pragma once

#include "IntegerSequence.hpp"
#include "Pair.hpp"

namespace hsd
{
    template < typename... T >
    class tuple
    {
    public:
        static constexpr usize size()
        {
            return sizeof...(T);
        }
    };

    template < typename... Args > static constexpr auto make_tuple(Args&&... args);

    template < typename Tuple1, typename Tuple2 >
    using is_same_tuple = std::is_same<Tuple1, Tuple2>;

    template < typename T, typename... Rest >
    class tuple<T, Rest...>
    {   
    public:   
        T _first;
        tuple<Rest...> _rest;        

    private:
        template < usize N, typename U >
        struct get_helper {};

        template < typename U, typename... Args >
        struct get_helper< 0, tuple<U, Args...> >
        {
            static constexpr U get(const tuple<U, Args...>& data)
            {
                return data._first;
            }
        };

        template < usize N, typename U, typename... Args >
        struct get_helper< N, tuple<U, Args...> >
        {
            static constexpr auto get(const tuple<U, Args...>& data)
            {
                return get_helper<N - 1, tuple<Args...>>::get(data._rest);
            }
        };

        template < usize N, typename... U >
        static constexpr auto _get(const tuple<U...>& tup) 
        {
            return get_helper<N, tuple<U...>>::get(tup);
        }
    public:
        tuple() = default;

        constexpr tuple(const T& first, const Rest&... rest) 
            : _first(first), _rest(rest...)
        {}

        constexpr tuple(const tuple& other)
            : _first{other._first}, _rest{other._rest}
        {}

        template < typename... Args >
        constexpr tuple& operator=(const tuple<Args...>& other)
        {
            _first = other._first;
            _rest = other._rest;

            return *this;
        }

        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) 
        {
            auto _add = [&]< usize... Ints1, usize... Ints2 >(
                index_sequence<Ints1...>, index_sequence<Ints2...>
            ) -> tuple<T, Rest..., Args...> {

                [](auto... args) {
                    static_assert(is_same_tuple<tuple<T, Rest..., Args...>, tuple<decltype(args)...>>{});
                }(get<Ints1>()..., rhs.template get<Ints2>()...);

                return make_tuple<T, Rest..., Args...>(
                    get<Ints1>()..., rhs.template get<Ints2>()...
                );
            };

            return _add(index_sequence_for<T , Rest...>{}, index_sequence_for<Args...>{});
        }

        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) const
        {
            auto _add = [&]< usize... Ints1, usize... Ints2 >(
                index_sequence<Ints1...>, index_sequence<Ints2...>
            ) -> tuple<T, Rest..., Args...> {

                [](auto... args) {
                    static_assert(is_same_tuple<tuple<T, Rest..., Args...>, tuple<decltype(args)...>>{});
                }(get<Ints1>()..., rhs.template get<Ints2>()...);

                return make_tuple<T, Rest..., Args...>(
                    get<Ints1>()..., rhs.template get<Ints2>()...
                );
            };

            return _add(index_sequence_for<T , Rest...>{}, index_sequence_for<Args...>{});
        }

        template <usize N>
        constexpr auto get() 
        {
            return _get<N, T, Rest...>(*this);
        }

        template <usize N>
        constexpr auto get() const
        {
            return _get<N, T, Rest...>(*this);
        }

        static constexpr usize size()
        {
            return 1 + sizeof...(Rest);
        }
    };

    template < typename... UTypes > tuple(UTypes...) -> tuple<UTypes...>;
    template < typename T1, typename T2 > tuple(pair<T1, T2>) -> tuple<T1, T2>;

    template <typename... Args>
    static constexpr auto make_tuple(Args&&... args)
    {
        return tuple<decay_t<Args>...>(hsd::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto tie(Args&... args)
    {
        return tuple<decay_t<Args>&...>(args...);
    }

    template < typename Func, typename... Args, usize... Is >
    static constexpr auto apply_impl(Func&& func, index_sequence<Is...>, const tuple<Args...>& args) 
    {
        return func(args.template get<Is>()...);
    }
    
    template < typename Func, typename... Args >
    static constexpr auto apply(Func&& func, const tuple<Args...>& args)
    {
      return apply_impl(hsd::forward<Func>(func), make_index_sequence<sizeof...(Args)>{}, args);
    }
}