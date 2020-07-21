#pragma once

#include "../Tuple/Tuple.hpp"
#include <stdexcept>

template< typename > class function;

template< typename Result, typename... Args >
class function< Result(Args...) > 
{
private:
    Result(*_func_impl)(Args...) = nullptr;

    struct _Useless {};

    tuple< Args... > _func_args;

    template< bool Condition, typename Value >
    using ResolvedType = typename std::enable_if< Condition, Value >::type;
public:
    function() = default;

    template< typename Func >
    constexpr function(Func&& function)
    {
        _func_impl = function;
    }

    constexpr function(const function& other)
    {
        _func_impl = other._func_impl;
    }

    constexpr function& operator=(const function& other)
    {
        _func_impl = other._func_impl;
    }

    template< typename Func >
    constexpr function& operator=(Func&& function)
    {
        _func_impl = function;
    }

    constexpr void bind(Args&&... arguments)
    {
        _func_args = make_tuple(arguments...);
    }

    constexpr Result operator()(Args&&... arguments)
    {
        if(_func_impl == nullptr)
        {
            throw std::runtime_error("Bad function");
        }
        
        return _func_impl(arguments...);
    }

    template< typename Value = Result >
    constexpr ResolvedType< (sizeof...(Args) != 0), Value > operator()()
    {
        if(_func_impl == nullptr)
        {
            throw std::runtime_error("Bad function");
        }
        
        return apply([&](auto&&... args){
            return _func_impl(args...);
        }, _func_args);
    }
};

template< typename T >
struct as_function
    : public as_function< decltype(&T::operator()) >
{};

template< typename R, typename... ArgTypes >
struct as_function< R(ArgTypes...) > 
{
    using type = R(ArgTypes...);
};

template< typename R, typename... ArgTypes >
struct as_function< R(&)(ArgTypes...) > 
{
    using type = R(ArgTypes...);
};

template< typename R, typename... ArgTypes >
struct as_function< R(&)(ArgTypes...) noexcept > 
{
    using type = R(ArgTypes...);
};

template< typename U, typename R, typename... ArgTypes >
struct as_function< R(U::*)(ArgTypes...) const > 
{
    using type = R(ArgTypes...);
};

template< typename Func > function(Func&&) -> function< typename as_function<Func>::type >;

namespace FuncTest
{
    static constexpr int func(int a) noexcept
    {
        return a;
    }

    static void Test()
    {
        //function f = [](){return 4;};
        function f = func;
        f.bind(5);
        printf("%d\n", f());
    }
}