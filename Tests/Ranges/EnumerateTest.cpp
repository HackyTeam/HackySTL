#include <cassert>
#include <ranges>
#include <vector>

#include <Containers/Span.hpp>
#include <Containers/Vector.hpp>

int main()
{
    hsd::vector<int> values{{10, 20, 30, 40, 50}};
    std::vector<int> standard_values{10, 20, 30, 40, 50};
    std::size_t expected_index = 0;

    for (auto [value, index] : values.to_span().enumerate())
    {
        assert(index == expected_index);
        assert(value == static_cast<int>((index + 1) * 10));
        value += 1;
        ++expected_index;
    }
    std::size_t standard_index = 0;
    for (auto [index, value] : standard_values | std::views::enumerate)
    {
        assert(index == static_cast<std::ptrdiff_t>(standard_index));
        assert(value == static_cast<int>((standard_index + 1) * 10));
        value += 1;
        ++standard_index;
    }

    assert(expected_index == values.size());
    assert(standard_index == standard_values.size());
    assert(values[0] == 11);
    assert(values[4] == 51);
    assert(values[0] == standard_values[0]);
    assert(values[4] == standard_values[4]);

    hsd::vector<int> empty_values;
    std::size_t empty_count = 0;
    for (auto [value, index] : empty_values.to_span().enumerate())
    {
        (void)value;
        (void)index;
        ++empty_count;
    }
    assert(empty_count == 0);
    std::vector<int> standard_empty_values;
    std::size_t standard_empty_count = 0;
    for (auto [index, value] : standard_empty_values | std::views::enumerate)
    {
        (void)value;
        (void)index;
        ++standard_empty_count;
    }
    assert(standard_empty_count == empty_count);

    std::size_t restarted_index = 0;
    for (auto [value, index] : values.to_span().enumerate())
    {
        (void)value;
        assert(index == restarted_index);
        ++restarted_index;
    }
    assert(restarted_index == values.size());
    standard_index = 0;
    for (auto [index, value] : standard_values | std::views::enumerate)
    {
        (void)value;
        assert(index == static_cast<std::ptrdiff_t>(standard_index));
        ++standard_index;
    }
    assert(standard_index == standard_values.size());
}
