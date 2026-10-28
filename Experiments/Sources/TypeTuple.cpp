#include <Concepts.hpp>

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
    template <hsd::usize Idx>
    using type_at = typename type_tuple_impl<Idx, Ts...>::type;
};

int main()
{
    using t = type_tuple<int, float, double>;

    static_assert(std::is_same<t::type_at<0>, int>::value);
    static_assert(std::is_same<t::type_at<1>, float>::value);
    static_assert(std::is_same<t::type_at<2>, double>::value);
}