#include <cassert>
#include <deque>
#include <vector>

#include <Containers/Deque.hpp>
#include <Containers/Vector.hpp>
#include <Support/TestSupport.hpp>

static std::vector<int> std_solution(
    const std::vector<int>& values, std::size_t window)
{
    std::deque<std::size_t> indices;
    std::vector<int> result;

    for (std::size_t index = 0; index < values.size(); ++index)
    {
        while (!indices.empty() && indices.front() + window <= index)
            indices.pop_front();
        while (!indices.empty() && values[indices.back()] <= values[index])
            indices.pop_back();
        indices.push_back(index);
        if (index + 1 >= window)
            result.push_back(values[indices.front()]);
    }
    return result;
}

static hsd::vector<int> hsd_solution(
    const hsd::vector<int>& values, std::size_t window)
{
    hsd::deque<std::size_t> indices;
    hsd::vector<int> result;

    for (std::size_t index = 0; index < values.size(); ++index)
    {
        while (indices.size() != 0 && indices.front() + window <= index)
            indices.pop_front();
        while (indices.size() != 0 && values[indices.back()] <= values[index])
            indices.pop_back();
        indices.push_back(index);
        if (index + 1 >= window)
            result.push_back(values[indices.front()]);
    }
    return result;
}

static void check_case(const std::vector<int>& input, std::size_t window)
{
    hsd::vector<int> values;
    for (const int value : input)
        values.push_back(value);

    const auto expected = std_solution(input, window);
    const auto actual = hsd_solution(values, window);
    test_support::expect_sequence(actual, expected);
}

int main()
{
    check_case({1, 3, -1, -3, 5, 3, 6, 7}, 3);
    check_case({9, 8, 7, 6, 5, 4}, 1);
    check_case({4, 4, 4, 4, 4}, 5);
    check_case({-5, -2, -8, -1, -3}, 2);
}