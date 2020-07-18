#pragma once

#include <tuple>
#include <stdexcept>

template< typename > class function;

template< typename Result, typename... Args >
class function< Result(Args...) > 
{
private:
    Result(*_func_impl)(Args...) = nullptr;

    struct _Useless {};

    std::tuple< Args... > _func_args;

    template< bool Condition, typename Value >
    using ResolvedType = typename std::enable_if< Condition, Value >::type;
public:
    function() = default;

    template< typename Func, ResolvedType< !std::is_integral< Func >{}, _Useless > = _Useless() >
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

    template< typename Func, typename std::enable_if< !std::is_integral< Func >{}, _Useless >::type = _Useless() >
    constexpr function& operator=(Func&& function)
    {
        _func_impl = function;
    }

    constexpr void bind(Args&&... arguments)
    {
        _func_args = std::make_tuple(arguments...);
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
        
        auto [_args] = std::apply(
            [](auto&&... args) {
                return std::forward_as_tuple(std::forward<decltype(args)>(args)...);
            }, _func_args);

        return _func_impl(_args);
    }
};

namespace FuncTest
{
    static void Test()
    {
        function<int()> f = [](){return 4;};
        //f.bind(4);
        auto var = f();
    }
}