#pragma once

#include "Tuple.hpp"
#include "Result.hpp"
#include "UniquePtr.hpp"

namespace hsd
{
    template <typename> class function;
    
    namespace func_detail
    {
        template < typename Func, typename Res, typename... Args >
        concept IsFunction = (
            !is_same<Func, function<Res(Args...)>>::value && 
            InvocableRet<Res, Func, Args...>
        );

        struct bad_function
        {
            const char* operator()() const
            {
                return "Bad function";
            }
        };

        template <typename T>
        struct store_ref
        {
            using type = T;
        };
        
        template <typename T>
        struct store_ref<T&>
        {
            using type = reference<T>;
        };

        template <typename T>
        using store_ref_t = typename store_ref<remove_cv_t<T>>::type;
    } // namespace func_detail

    template < typename ResultType, typename... Args >
    class function<ResultType(Args...)> 
    {
    private:
        struct callable_base
        {
            virtual unique_ptr<callable_base> clone() const = 0;
            virtual ResultType operator()(Args&&...) = 0;
            virtual ~callable_base() = default;
        };

        template <typename Func>
        struct callable : callable_base
        {
            Func _func;
            callable(Func func)
                : _func{func}
            {}

            virtual ResultType operator()(Args&&... args) override
            {
                if constexpr (requires {{_func(forward<Args>(args)...)} -> IsSame<ResultType>;})
                {
                    return _func(forward<Args>(args)...);
                }
                else if constexpr (UnwrapInvocable<Func, Args...>)
                {
                    return _func(forward<Args>(args)...).unwrap();
                }
            }

            virtual unique_ptr<callable_base> clone() const override
            {
                return make_unique<callable<Func>>(_func);
            }
        };
        
        unique_ptr<callable_base> _func_impl{};

        inline void reset()
        {
            _func_impl = nullptr;
        }

    public:
        inline function() = default;
        inline function(NullType) {}

        inline function(auto func)
        requires (func_detail::IsFunction<decltype(func), ResultType, Args...>);

        inline function(const function&);

        inline function(function&& other)
        {
            swap(_func_impl, other._func_impl);
        }

        inline ~function()
        {
            reset();
        }

        inline function& operator=(const function& other);

        inline function& operator=(auto func)
        requires (func_detail::IsFunction<decltype(func), ResultType, Args...>);

        inline function& operator=(function&& other)
        {
            swap(_func_impl, other._func_impl);
            return *this;
        }

        inline function& operator=(NullType)
        {
            reset();
            return *this;
        }

        inline explicit operator bool() const
        {
            return _func_impl.get() != nullptr;
        }

        inline auto operator()(Args... args) 
            -> Result<
                func_detail::store_ref_t<ResultType>, 
                func_detail::bad_function >
        requires (!is_void<ResultType>::value)
        {
            if (_func_impl == nullptr)
            {
                return func_detail::bad_function{};
            }

            return {_func_impl->operator()(forward<Args>(args)...)};
        }

        inline auto operator()(Args... args) 
            -> Result< void, func_detail::bad_function >
        requires (is_void<ResultType>::value)
        {
            if (_func_impl == nullptr)
            {
                return func_detail::bad_function{};
            }

            _func_impl->operator()(forward<Args>(args)...);
            return {};
        }

        inline auto operator()(Args... args) const
            -> Result<
                const func_detail::store_ref_t<ResultType>, 
                func_detail::bad_function >
        requires (!is_void<ResultType>::value)
        {
            if (_func_impl == nullptr)
            {
                return func_detail::bad_function{};
            }

            return {_func_impl->operator()(forward<Args>(args)...)};
        }

        inline auto operator()(Args... args) const
            -> Result< void, func_detail::bad_function >
        requires (is_void<ResultType>::value)
        {
            if (_func_impl == nullptr)
            {
                return func_detail::bad_function{};
            }

            _func_impl->operator()(forward<Args>(args)...);
            return {};
        }
    };

    namespace functional_helper
    {
        template <typename>
        struct as_function {};

        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) noexcept(Case)>
        {
            using type = Res(Args...);
        };

        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...)& noexcept(Case)>
        { 
            using type = Res(Args...);
        };

        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) const noexcept(Case)>
        { 
            using type = Res(Args...);
        };

        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) const& noexcept(Case)>
        { 
            using type = Res(Args...);
        };
    } // namespace functional_helper

    template < typename Res, typename... Args >
    inline function<Res(Args...)>::function(const function& other)
    {
        _func_impl = other._func_impl->clone();
    }

    template < typename Res, typename... Args >
    inline function<Res(Args...)>::function(auto func)
    requires (func_detail::IsFunction<decltype(func), Res, Args...>)
    {
        _func_impl = make_unique<callable<decltype(func)>>(func);
    }

    template < typename Res, typename... Args >
    inline auto function<Res(Args...)>::operator=(const function<Res(Args...)>& other)
        -> function<Res(Args...)>&
    {
        reset();
        _func_impl = other._func_impl->clone();
        return *this;
    }

    template < typename Res, typename... Args >
    inline auto function<Res(Args...)>::operator=(auto func)
        -> function<Res(Args...)>&
    requires (func_detail::IsFunction<decltype(func), Res, Args...>)
    {
        using func_type = decltype(func);

        reset();
        _func_impl = make_unique<callable<func_type>>(forward<func_type>(func));
        return *this;
    }

    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T&& value, Args&&... args)
    {
        return [=, value{move(value)}]() mutable {
            if constexpr (requires {value.*(func(declval<Args>()...)).unwrap();})
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...).unwrap();
                }
                else
                {
                    return (value.*func)(args...).unwrap();
                }
            }
            else
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...);
                }
                else
                {
                    return (value.*func)(args...);
                }
            }
        };
    }

    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T&& value, hsd::tuple<Args...>&& args)
    {
        return [=, value{move(value)}]() mutable {
            return [&]<usize... Ints>(hsd::index_sequence<Ints...>)
            {
                if constexpr (requires {(value.*func)(declval<Args>()...).unwrap();})
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                }
                else
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        );
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        );
                    }
                }
            }(hsd::index_sequence_for<Args...>{});
        };
    }

    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T& value, Args&&... args)
    {
        return [=, &value]{
            if constexpr (requires {value.*(func(declval<Args>()...)).unwrap();})
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...).unwrap();
                }
                else
                {
                    return (value.*func)(args...).unwrap();
                }
            }
            else
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...);
                }
                else
                {
                    return (value.*func)(args...);
                }
            }
        };
    }

    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T& value, hsd::tuple<Args...>&& args)
    {
        return [=, &value]{
            return [&]<usize... Ints>(hsd::index_sequence<Ints...>)
            {
                if constexpr (requires {(value.*func)(declval<Args>()...).unwrap();})
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                }
                else
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        );
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        );
                    }
                }
            }(hsd::index_sequence_for<Args...>{});
        };
    }

    template < typename Func, typename... Args >
    requires (!std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, Args&&... args)
    {
        return [=]{
            if constexpr (requires {func(declval<Args>()...).unwrap();})
            {
                return func(args...).unwrap();
            }
            else
            {
                return func(args...);
            }
        };
    }

    template < typename Func, typename... Args >
    requires (!std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, hsd::tuple<Args...>&& args)
    {
        return [=]{
            return [&]<usize... Ints>(hsd::index_sequence<Ints...>)
            {
                if constexpr (requires {func(declval<Args>()...).unwrap();})
                {
                    return func(args.template get<Ints>()...).unwrap();
                }
                else
                {
                    return func(args.template get<Ints>()...);
                }
            }(hsd::index_sequence_for<Args...>{});
        };
    }

    template < typename Res, typename... Args > 
    function(Res(*)(Args...)) -> function<Res(Args...)>;
    
    template < typename Func, typename Op = decltype(&Func::operator()) > 
    function(Func) -> function<
        typename functional_helper::as_function<Op>::type
    >;
} // namespace hsd