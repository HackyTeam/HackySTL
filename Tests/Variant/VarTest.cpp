#include <cassert>
#include <variant>

#include <Wrappers/Variant.hpp>

struct text_value
{
    int value;

    friend bool operator==(const text_value&, const text_value&) = default;
};

static void test_type_access()
{
    hsd::variant<int, float, text_value> value{12};
    std::variant<int, float, text_value> standard_value{12};
    assert(value.index() == 0);
    assert(value.index() == standard_value.index());
    assert(value.holds<int>());
    assert(!value.holds<float>());
    assert(value.get<int>().unwrap().get() == 12);
    assert(value.get<int>().unwrap().get() == std::get<int>(standard_value));
    assert(value.get_if<float>() == nullptr);

    value = 2.5f;
    standard_value = 2.5f;
    assert(value.index() == 1);
    assert(value.index() == standard_value.index());
    assert(value.holds<float>());
    assert(value.get<float>().unwrap().get() == 2.5f);
    assert(value.get<float>().unwrap().get() == std::get<float>(standard_value));
    assert(!value.get<int>().is_ok());
    assert(value.get_if<int>() == nullptr);

    value = text_value{77};
    standard_value = text_value{77};
    assert(value.index() == 2);
    assert(value.index() == standard_value.index());
    assert(value.get<text_value>().unwrap().get().value == 77);
    assert(value.get<text_value>().unwrap().get().value
        == std::get<text_value>(standard_value).value);
}

static void test_copy_move_and_visit()
{
    hsd::variant<int, float, text_value> source{42};
    std::variant<int, float, text_value> standard_source{42};
    hsd::variant<int, float, text_value> copy{source};
    std::variant<int, float, text_value> standard_copy{standard_source};
    assert(copy == source);
    assert(copy.index() == standard_copy.index());
    assert(copy.get<int>().unwrap().get() == std::get<int>(standard_copy));

    copy = text_value{8};
    standard_copy = text_value{8};
    assert(copy != source);
    assert(copy.get<text_value>().unwrap().get().value
        == std::get<text_value>(standard_copy).value);
    assert(copy.visit([](const auto& value)
    {
        if constexpr (hsd::IsSame<hsd::remove_cvref_t<decltype(value)>, text_value>)
        {
            return value.value;
        }
        else
        {
            return 0;
        }
    }).unwrap() == 8);
    assert(std::visit([](const auto& value)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(value)>, text_value>)
        {
            return value.value;
        }
        else
        {
            return 0;
        }
    }, standard_copy) == 8);

    hsd::variant<int, float, text_value> moved{hsd::move(copy)};
    std::variant<int, float, text_value> standard_moved{std::move(standard_copy)};
    assert(moved.holds<text_value>());
    assert(moved.get<2>().unwrap().get().value == 8);
    assert(moved.index() == standard_moved.index());
    assert(moved.get<text_value>().unwrap().get().value
        == std::get<text_value>(standard_moved).value);

    int visited = 0;
    moved.visit([&](const auto& value)
    {
        if constexpr (hsd::IsSame<hsd::remove_cvref_t<decltype(value)>, text_value>)
        {
            visited = value.value;
        }
    }).unwrap();
    assert(visited == 8);
}

static void test_in_place_and_const_access()
{
    hsd::variant<int, float, text_value> value{
        hsd::in_place_t<text_value>{}, 19
    };
    const auto& const_value = value;
    assert(const_value.get_if<text_value>()->value == 19);
    assert(const_value.get<text_value>().unwrap().get().value == 19);

    value = 7;
    assert(value.holds<int>() && value.get_if<int>() != nullptr);

    hsd::variant<int, float> defaults;
    assert(defaults.index() == 0);
    assert(defaults.get<0>().unwrap().get() == 0);
}

int main()
{
    test_type_access();
    test_copy_move_and_visit();
    test_in_place_and_const_access();
}
