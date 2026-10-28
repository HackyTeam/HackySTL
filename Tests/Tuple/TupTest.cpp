#include <cassert>
#include <tuple>

#include <Wrappers/Tuple.hpp>

int main()
{
    constexpr hsd::tuple values = {500, 'a', "tuple"};
    constexpr auto standard_values = std::tuple{500, 'a', "tuple"};
    static_assert(values.get<0>() == 500);
    static_assert(values.get<1>() == 'a');
    static_assert(values.get<2>()[0] == 't');
    static_assert(values.get<0>() == std::get<0>(standard_values));
    static_assert(values.get<1>() == std::get<1>(standard_values));
    static_assert(values.get<2>()[0] == std::get<2>(standard_values)[0]);

    constexpr auto made = hsd::make_tuple(-1, static_cast<hsd::u64>(9), 1.25f);
    constexpr auto standard_made = std::make_tuple(-1, static_cast<hsd::u64>(9), 1.25f);
    static_assert(made.get<0>() == -1);
    static_assert(made.get<1>() == 9);
    static_assert(made.get<2>() == 1.25f);
    static_assert(made.get<0>() == std::get<0>(standard_made));
    static_assert(made.get<1>() == std::get<1>(standard_made));
    static_assert(made.get<2>() == std::get<2>(standard_made));

    auto combined = values + made;
    auto standard_combined = std::tuple_cat(standard_values, standard_made);
    assert(combined.get<0>() == 500);
    assert(combined.get<1>() == 'a');
    assert(combined.get<3>() == -1);
    assert(combined.get<5>() == 1.25f);
    assert(combined.get<0>() == std::get<0>(standard_combined));
    assert(combined.get<1>() == std::get<1>(standard_combined));
    assert(combined.get<2>()[0] == std::get<2>(standard_combined)[0]);
    assert(combined.get<3>() == std::get<3>(standard_combined));
    assert(combined.get<4>() == std::get<4>(standard_combined));
    assert(combined.get<5>() == std::get<5>(standard_combined));

    int number = 12;
    auto refs = hsd::tie(number);
    auto standard_refs = std::tie(number);
    refs.get<0>() = 99;
    std::get<0>(standard_refs) = 99;
    assert(number == 99);

    auto copied = values;
    assert(copied.get<0>() == values.get<0>());
    assert(copied.get<1>() == values.get<1>());
    constexpr auto standard_tuple_size = std::tuple_size_v<std::tuple<int, char>>;
    static_assert(standard_tuple_size == 2);
}
