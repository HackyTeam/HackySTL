#pragma once

// Requires C++20

#include "../Functional.hpp"

namespace hsd
{
    struct default_t {};
    constexpr default_t default_v{};

    template <typename S, typename F>
    struct defaultcall_t;

    template <typename F, typename Result, typename... Args>
    struct defaultcall_t<Result(Args...), F>;

    template <usize Id, typename F, typename Result, typename... Args, typename T>
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const&, T&& x);
    template <usize Id, typename F, typename Result, typename... Args>
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const& c, default_t);

    template <typename F, typename Result, typename... Args>
    struct defaultcall_t<Result(Args...), F>
    {
        tuple<Args...> default_args;
        F func;
        constexpr defaultcall_t(F&& func, Args&&... args) 
            : default_args(forward<Args>(args)...), func(forward<F>(func)) 
        {}

        template <typename... U>
        constexpr Result operator()(U&&... args) const 
        {
            // Because GCC is stupid and doesn't support proper capturing
            return [this]<usize... Seq>(index_sequence<Seq...>, auto&&... args) 
            {
                #define default_cast_func default_cast<Seq, F, Result, Args...>
                
                if constexpr (UnwrapInvocable<
                    F, decltype(default_cast_func(*this, forward<U>(args)))...
                >)
                {
                    return func(default_cast_func(*this, forward<U>(args))...).unwrap();
                }
                else
                {
                    return func(default_cast_func(*this, forward<U>(args))...);
                }

                #undef default_cast_func
            }(index_sequence_for<Args...>{}, forward<U>(args)...);
        }
    };  

    template <typename S, typename F, typename... Args>
    defaultcall_t<S, F> make_defaultcall(F&& func, Args&&... args)
    {
        return defaultcall_t<S, F>(forward<F>(func), forward<Args>(args)...);
    }

    template <usize Id, typename F, typename Result, typename... Args, typename T>
    requires (!IsSame<T, default_t>)
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const&, T&& x) 
    {
        return forward<T>(x);
    }

    template <usize Id, typename F, typename Result, typename... Args>
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const& c, default_t)
    {
        return c.default_args.template get<Id>();
    }

    template <typename Res, typename... Args>
    defaultcall_t(Res(*)(Args...), Args&&...)
        -> defaultcall_t<Res(Args...), Res(*)(Args...)>;

    template <typename Res, typename... Args>
    defaultcall_t(Res(&)(Args...), Args&&...)
        -> defaultcall_t<Res(Args...), Res(&)(Args...)>;

    template <typename Res, typename... Args> 
    defaultcall_t(function<Res(Args...)>&&, Args&&...)
        -> defaultcall_t<Res(Args...), function<Res(Args...)>>;

    template < typename Func, typename... Args, 
        typename Op = decltype(&remove_reference_t<Func>::operator()) > 
    defaultcall_t(Func&&, Args&&...)
        -> defaultcall_t<typename functional_helper::as_function<Op>::type, Func>;
}
