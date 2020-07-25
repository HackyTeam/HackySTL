#pragma once

// Requires C++ 17

#include "../Tuple/Tuple.hpp"
#include "../Functional/Functional.hpp"

namespace hsd
{
    struct default_t {};
    constexpr default_t default_v{};

    template<typename S, typename F>
    struct defaultcall_t;

    template <typename F, typename Result, typename... Args>
    struct defaultcall_t<Result(Args...), F>;

    template <size_t Id, typename F, typename Result, typename... Args, typename T>
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const&, T&& x);
    template <size_t Id, typename F, typename Result, typename... Args>
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const& c, default_t);

    template <typename F, typename Result, typename... Args>
    struct defaultcall_t<Result(Args...), F>{
        tuple<Args...> default_args;
        F func;
        constexpr defaultcall_t(F&& func, Args&&... args) 
            : default_args(hsd::forward<Args>(args)...), func(hsd::forward<F>(func)) 
        {}

        private:
        template <typename... U, size_t... Seq>
        constexpr Result operator()(index_sequence<Seq...>, U&&... args) const {
            return func(default_cast<Seq, F, Result, Args...>(*this, hsd::forward<U>(args))...);
        }

        public:
        template <typename... U>
        constexpr Result operator()(U&&... args) const {
            return operator()(index_sequence_for<Args...>(), hsd::forward<U>(args)...);
        }

    };  

    template<typename S, typename F, typename... Args>
    defaultcall_t<S, F> make_defaultcall(F&& func, Args&&... args){
        return defaultcall_t<S, F>(hsd::forward<F>(func), hsd::forward<Args>(args)...);
    }

    template <size_t Id, typename F, typename Result, typename... Args, typename T>
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const&, T&& x) {
        return hsd::forward<T>(x);
    }

    template <size_t Id, typename F, typename Result, typename... Args>
    constexpr auto default_cast(defaultcall_t<Result(Args...), F> const& c, default_t){
        return c.default_args.template get<Id>();
    }

    template <typename F, typename... Args, 
        typename Op = decltype(&hsd::rev_ref_t<F>::operator())>
    defaultcall_t(F&&, Args&&...)
        -> defaultcall_t<typename hsd::helper::as_function<Op>::type, F>;

    template <typename Rez, typename... Args>
    defaultcall_t(Rez(*)(Args...), Args&&...)
        -> defaultcall_t<Rez(Args...), Rez(*)(Args...)>;

    template <typename Rez, typename... Args>
    defaultcall_t(Rez(&)(Args...), Args&&...)
        -> defaultcall_t<Rez(Args...), Rez(&)(Args...)>;
}
