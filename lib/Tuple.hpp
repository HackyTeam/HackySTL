#pragma once

#include "Reference.hpp"
#include "Pair.hpp"

namespace hsd
{
    template <typename... T>
    class tuple;

    template <typename... Args>
    static constexpr auto make_tuple(Args&&... args)
        -> tuple<decay_t<Args>...>;

    template < typename Tuple1, typename Tuple2 >
    using is_same_tuple = is_same<Tuple1, Tuple2>;

    template <typename>
    struct is_tuple : false_type {};

    template <typename... Args>
    struct is_tuple<tuple<Args...>> : true_type {};

    template <typename T>
    concept IsTuple = is_tuple<T>::value;

    template <>
    class tuple<>
    {
    public:
        template <typename... U>
        friend class tuple;

        static consteval usize size()
        {
            return 0;
        }

        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) const
        {
            return rhs;
        }

        template <typename U> requires (!is_tuple<U>::value)
        constexpr auto operator+(const U& rhs) const
        {
            return make_tuple(forward<U>(rhs));
        }
    };

    template <typename T> requires (!IsReference<T>)
    class tuple<T>
    {   
    private:   
        T _first;       

        template <typename... U>
        friend class tuple;

        template <usize N> requires (N == 0)
        constexpr const auto& get_raw() const
        {
            return _first;
        }
    public:
        constexpr tuple() = default;

        template <typename U> requires (Constructible<T, U>)
        constexpr tuple(U&& first) 
            : _first{forward<U>(first)}
        {}

        constexpr tuple(const tuple& other)
            : _first{other._first}
        {}

        constexpr tuple(tuple&& other)
            : _first{move(other._first)}
        {}

        constexpr tuple& operator=(const tuple& other)
        {
            _first = other._first;
            return *this;
        }

        constexpr tuple& operator=(tuple&& other)
        {
            _first = move(other._first);
            return *this;
        }

        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) const
        {
            return [&]<usize... Ints>(index_sequence<Ints...>)
            {
                return []<typename... TupArgs>(TupArgs... args)
                {
                    return make_tuple<TupArgs...>(move(args)...);
                }(get_raw<0>(), rhs.template get_raw<Ints>()...);
            }(index_sequence_for<Args...>{});
        }

        template <typename U> requires (!is_tuple<U>::value)
        constexpr auto operator+(const U& rhs) const
        {
            return []<typename... TupArgs>(TupArgs... args)
            {
                return make_tuple<TupArgs...>(move(args)...);
            }(get_raw<0>(), rhs);
        }

        template <usize N> requires (N == 0)
        constexpr auto& get()
        {
            if constexpr (requires {{_first.get()} -> IsReference;})
            {
                return _first.get();
            }
            else
            {
                return _first;
            }
        }

        template <usize N> requires (N == 0)
        constexpr const auto& get() const
        {
            if constexpr (requires {{_first.get()} -> IsReference;})
            {
                return _first.get();
            }
            else
            {
                return _first;
            }
        }

        static consteval usize size()
        {
            return 1;
        }
    };

    template <typename T> requires (IsReference<T>)
    class tuple<T>
        : public tuple<reference<remove_reference_t<T>>>
    {};

    template < typename T, typename... Rest > requires (!IsReference<T>)
    class tuple<T, Rest...>
    {   
    private:   
        T _first;
        tuple<Rest...> _rest;  

        template <typename... U>
        friend class tuple;      

        template <usize N>
        constexpr const auto& get_raw() const
        {
            if constexpr (N == 0)
            {
                return _first;
            }
            else
            {
                return _rest.template get_raw<N - 1>();
            }
        }
    public:
        constexpr tuple() = default;

        template <typename U, typename... Args> 
        requires (Constructible<T, U> && Constructible<tuple<Rest...>, Args...>)
        constexpr tuple(U&& first, Args&&... rest) 
            : _first{forward<U>(first)}, _rest{forward<Args>(rest)...}
        {}

        constexpr tuple(const tuple& other)
            : _first{other._first}, _rest{other._rest}
        {}

        constexpr tuple(tuple&& other)
            : _first{move(other._first)}, _rest{move(other._rest)}
        {}

        constexpr tuple& operator=(const tuple& other)
        {
            _first = other._first;
            _rest = other._rest;
            return *this;
        }

        constexpr tuple& operator=(tuple&& other)
        {
            _first = move(other._first);
            _rest = move(other._rest);
            return *this;
        }

        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) const
        {
            return [&]< usize... Ints1, usize... Ints2 >
                (index_sequence<Ints1...>, index_sequence<Ints2...>)
            {
                return []<typename... TupArgs>(TupArgs... args)
                {
                    return make_tuple<TupArgs...>(move(args)...);
                }(get_raw<Ints1>()..., rhs.template get_raw<Ints2>()...);
            }(index_sequence_for<T, Rest...>{}, index_sequence_for<Args...>{});
        }

        template <typename U> requires (!is_tuple<U>::value)
        constexpr auto operator+(const U& rhs) const
        {
            return [&]< usize... Ints1>
                (index_sequence<Ints1...>)
            {
                return []<typename... TupArgs>(TupArgs... args)
                {
                    return make_tuple<TupArgs...>(move(args)...);
                }(get_raw<Ints1>()..., rhs);
            }(index_sequence_for<T, Rest...>{});
        }

        template <usize N>
        constexpr auto& get()
        {
            if constexpr (N == 0)
            {
                if constexpr (requires {{_first.get()} -> IsReference;})
                {
                    return _first.get();
                }
                else
                {
                    return _first;
                }
            }
            else
            {
                return _rest.template get<N - 1>();
            }
        }

        template <usize N>
        constexpr const auto& get() const
        {
            if constexpr(N == 0)
            {
                if constexpr (requires {{_first.get()} -> IsReference;})
                {
                    return _first.get();
                }
                else
                {
                    return _first;
                }
            }
            else
            {
                return _rest.template get<N - 1>();
            }
        }

        static consteval usize size()
        {
            return 1 + sizeof...(Rest);
        }
    };

    template <typename T, typename... Rest> requires (IsReference<T>)
    class tuple<T, Rest...>
        : public tuple<reference<remove_reference_t<T>>, Rest...>
    {};

    template <hsd::usize, typename...>
    struct type_tuple_impl;

    template <typename T, typename... Ts>
    struct type_tuple_impl<0, T, Ts...>
    {
        using type = T;
    };

    template <hsd::usize Idx, typename T, typename... Ts>
    struct type_tuple_impl<Idx, T, Ts...>
    {
        using next = type_tuple_impl<Idx - 1, Ts...>;
        using type = typename next::type;
    };

    template <typename... Ts>
    struct type_tuple
    {
        static constexpr usize size = sizeof...(Ts);

        template <hsd::usize Idx>
        using type_at = typename type_tuple_impl<Idx, Ts...>::type;
    };

    template <typename... Ts>
    struct type_tuple<tuple<Ts...>>
        : type_tuple<Ts...>
    {};

    template < typename... UTypes > tuple(UTypes...) -> tuple<UTypes...>;
    template < typename T1, typename T2 > tuple(pair<T1, T2>) -> tuple<T1, T2>;

    template <typename... Args>
    static constexpr auto make_tuple(Args&&... args)
        -> tuple<decay_t<Args>...>
    {
        return {forward<decay_t<Args>>(args)...};
    }

    template <typename... Args>
    static constexpr auto tie(Args&... args)
        -> tuple<decay_t<Args>&...>
    {
        return {forward<decay_t<Args>&>(args)...};
    }

    template < typename Func, typename T, typename... Args, usize... Is >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply_impl(
        Func&& func, T&& value, index_sequence<Is...>, const tuple<Args...>& args) 
    {
        if constexpr (is_pointer<T>::value)
        {
            return (value->*func)(args.template get<Is>()...);
        }
        else
        {
            return (value.*func)(args.template get<Is>()...);
        }
    }
    
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply(Func&& func, T&& value, const tuple<Args...>& args)
    {
        return apply_impl(
            forward<Func>(func), forward<T>(value), 
            make_index_sequence<sizeof...(Args)>{}, args
        );
    }

    template < typename Func, typename T, typename... Args, usize... Is >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply_impl(
        Func&& func, T& value, index_sequence<Is...>, const tuple<Args...>& args) 
    {
        if constexpr (is_pointer<T>::value)
        {
            return (value->*func)(args.template get<Is>()...);
        }
        else
        {
            return (value.*func)(args.template get<Is>()...);
        }
    }
    
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply(Func&& func, T& value, const tuple<Args...>& args)
    {
        return apply_impl(
            forward<Func>(func), value, make_index_sequence<sizeof...(Args)>{}, args
        );
    }

    template < typename Func, typename... Args, usize... Is >
    requires (!std::is_member_function_pointer_v<Func>)
    static constexpr auto apply_impl(
        Func&& func, index_sequence<Is...>, const tuple<Args...>& args) 
    {
        return func(args.template get<Is>()...);
    }
    
    template < typename Func, typename... Args >
    requires (!std::is_member_function_pointer_v<Func>)
    static constexpr auto apply(Func&& func, const tuple<Args...>& args)
    {
        return apply_impl(
            forward<Func>(func), make_index_sequence<sizeof...(Args)>{}, args
        );
    }
}