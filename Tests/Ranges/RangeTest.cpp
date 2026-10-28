#include <cassert>
#include <ranges>
#include <vector>

#include <Containers/Range.hpp>
#include <Containers/Vector.hpp>
#include <Support/TestSupport.hpp>

static void test_views()
{
    hsd::vector<int> values{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};
    const std::vector<int> standard_values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::vector<int> dropped;
    for (const auto& value : values | hsd::views::drop(3))
    {
        dropped.push_back(value);
    }
    const std::vector<int> standard_dropped(
        standard_values | std::views::drop(3) | std::ranges::to<std::vector<int>>()
    );
    test_support::expect_sequence(dropped, standard_dropped);

    std::vector<int> taken;
    for (const auto& value : values | hsd::views::take(4))
    {
        taken.push_back(value);
    }
    const std::vector<int> standard_taken(
        standard_values | std::views::take(4) | std::ranges::to<std::vector<int>>()
    );
    test_support::expect_sequence(taken, standard_taken);

    std::vector<int> dropped_while;
    for (const auto& value : values | hsd::views::drop_while([](int value)
    {
        return value < 5;
    }))
    {
        dropped_while.push_back(value);
    }
    const std::vector<int> standard_dropped_while(
        standard_values
        | std::views::drop_while([](int value) { return value < 5; })
        | std::ranges::to<std::vector<int>>()
    );
    test_support::expect_sequence(dropped_while, standard_dropped_while);

    std::vector<int> taken_while;
    for (const auto& value : values | hsd::views::take_while([](int value)
    {
        return value < 5;
    }))
    {
        taken_while.push_back(value);
    }
    const std::vector<int> standard_taken_while(
        standard_values
        | std::views::take_while([](int value) { return value < 5; })
        | std::ranges::to<std::vector<int>>()
    );
    test_support::expect_sequence(taken_while, standard_taken_while);
}

static void test_adaptors()
{
    hsd::vector<int> values{{1, 2, 3, 4, 5, 6}};
    const std::vector<int> standard_values{1, 2, 3, 4, 5, 6};
    std::vector<int> transformed;

    for (const auto& value : values
        | hsd::ranges::filter([](int value) { return value % 2 == 0; })
        | hsd::ranges::transform([](int value) { return value * value; }))
    {
        transformed.push_back(value);
    }
    const std::vector<int> standard_transformed(
        standard_values
        | std::views::filter([](int value) { return value % 2 == 0; })
        | std::views::transform([](int value) { return value * value; })
        | std::ranges::to<std::vector<int>>()
    );
    test_support::expect_sequence(transformed, standard_transformed);

    std::vector<int> reversed;
    for (const auto& value : values | hsd::views::reverse)
    {
        reversed.push_back(value);
    }
    const std::vector<int> standard_reversed(
        standard_values | std::views::reverse | std::ranges::to<std::vector<int>>()
    );
    test_support::expect_sequence(reversed, standard_reversed);

    for (auto& value : values | hsd::ranges::filter([](int value)
    {
        return value % 2 == 1;
    }))
    {
        value *= 10;
    }
    std::vector<int> standard_mutated{1, 2, 3, 4, 5, 6};
    for (auto& value : standard_mutated | std::views::filter([](int value)
    {
        return value % 2 == 1;
    }))
    {
        value *= 10;
    }
    test_support::expect_sequence(values, standard_mutated);
}

int main()
{
    test_views();
    test_adaptors();
}
