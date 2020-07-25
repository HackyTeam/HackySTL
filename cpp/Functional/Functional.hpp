#pragma once

#include "../Tuple/Tuple.hpp"
#include <stdexcept>

namespace hsd
{
    using nullptr_t = decltype(nullptr);
    template< typename > class function;

    template< typename Result, typename... Args >
    class function< Result(Args...) > 
    {
    private:
        struct callable_base
        {
            size_t _instances = 1;
            virtual Result operator()(Args&&...) = 0;
            virtual ~callable_base() {}
        };

        template<typename Func>
        struct callable : callable_base
        {
            Func _func;
            callable(Func func)
                : _func{func}
            {}

            virtual Result operator()(Args&&... args) override
            {
                return _func(hsd::forward<Args>(args)...);
            }
        };
        
        callable_base* _func_impl = nullptr;
        template< typename Condition, typename Value >
        using ResolvedType = typename std::enable_if< Condition::value, Value >::type;

        constexpr void reset()
        {
            if(_func_impl != nullptr)
            {
                if(_func_impl->_instances == 1)
                {
                    delete _func_impl;
                    _func_impl = nullptr;
                }
                else
                    _func_impl->_instances--;
            }
        }
    public:
        function() = default;

        template< typename Func, 
            typename = ResolvedType<std::__not_<std::is_same<Func, function>>, void>,
            typename = ResolvedType<std::is_invocable<Func, Args...>, void>> 
        constexpr function(Func);

        constexpr function(const function&);

        constexpr function(function&& other)
        {
            hsd::swap(_func_impl, other._func_impl);
        }

        constexpr function(nullptr_t){}

        constexpr ~function()
        {
            reset();
        }

        constexpr function& operator=(const function& other)
        {
            reset();
            _func_impl = other._func_impl;
            if(_func_impl != nullptr)
                _func_impl->instances++;
            return *this;
        }

        constexpr function& operator=(function&& other)
        {
            hsd::swap(_func_impl, other._func_impl);
            return *this;
        }

        template<typename Func>
        constexpr function& operator=(Func func)
        {
            if(static_cast<bool>(func))
            {
                reset();
                _func_impl = new callable<Func>(func);
            }
            return *this;
        }

        constexpr function& operator=(nullptr_t)
        {
            reset();
        }

        constexpr Result operator()(Args&&... args)
        {
            if(_func_impl == nullptr)
            {
                throw std::runtime_error("Bad function");
            }

            return _func_impl->operator()(hsd::forward<Args>(args)...);
        }
    };

    namespace helper
    {
        template<typename>
        struct as_function {};

        template<typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) noexcept(Case)>
        {
            using type = Res(Args...);
        };

        template<typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...)& noexcept(Case)>
        { 
            using type = Res(Args...);
        };

        template<typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) const noexcept(Case)>
        { 
            using type = Res(Args...);
        };

        template<typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) const& noexcept(Case)>
        { 
            using type = Res(Args...);
        };
    }

    template< typename Res, typename... Args >
    constexpr function<Res(Args...)>::function(const function& other)
    {
        _func_impl = other._func_impl;
        if(_func_impl != nullptr)
            _func_impl->_instances++;
    }

    template< typename Res, typename... Args >
    template< typename Func, typename, typename >
    constexpr function<Res(Args...)>::function(Func func)
    {
        _func_impl = new callable<Func>(func);
    }

    template< typename Func, typename... Args >
    static constexpr auto bind(Func func, Args&&... args)
    {
        return [&, func]{
            hsd::function _internal_func = func;
            return _internal_func(hsd::forward<Args>(args)...);
        };
    }

    template< typename Func, typename... Args >
    static constexpr auto bind(Func func, hsd::tuple<Args...> args)
    {
        return [&, func]{
            return [&, func]<size_t... Ints>(hsd::index_sequence<Ints...>)
            {
                hsd::function _internal_func = func;
                return _internal_func(hsd::forward<Args>(args.template get<Ints>())...);
            }(hsd::make_index_sequence<args.size()>{});
        };
    }

    template < class Rez, class... Args > function(Rez(*)(Args...)) -> function<Rez(Args...)>;
    template< typename Func, typename Op = decltype(&Func::operator()) > 
        function(Func) -> function<typename helper::as_function<Op>::type>;
}