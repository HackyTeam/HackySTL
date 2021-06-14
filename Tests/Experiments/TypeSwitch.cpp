#include <TypeTraits.hpp>

template <auto _Value, typename _Ty>
struct ValueTypePair {
    constexpr static auto value = _Value;
    using type = _Ty;
};

template <auto _V, auto... _Cases>
struct ConstexprTypeSwitch
{
    using type = void;
};

template <auto _V, auto _Case1, auto... _Cases>
requires (_V == _Case1.value)
struct ConstexprTypeSwitch<_V, _Case1, _Cases...> 
{
    using type = typename decltype(_Case1)::type;
};

template <auto _V, auto _Case1, auto... _Cases>
requires (_V != _Case1.value)
struct ConstexprTypeSwitch<_V, _Case1, _Cases...> 
    : ConstexprTypeSwitch<_V, _Cases...> {};

template <auto _V, auto... _Cases>
using ConstexprSwitch_t = typename ConstexprTypeSwitch<_V, _Cases...>::type;

int main()
{
    static_assert(hsd::is_same<
        hsd::i32, ConstexprSwitch_t<
        2,
        ValueTypePair<0, hsd::f128>{},
        ValueTypePair<1, hsd::u64>{},
        ValueTypePair<2, hsd::i32>{}
        >
    >::value);
}