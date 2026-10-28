#include <algorithm>
#include <cassert>
#include <vector>

#include <Containers/Vector.hpp>

#include <Support/TestSupport.hpp>

struct counted_value
{
    static inline int alive = 0;

    int value = 0;

    explicit counted_value(int initial_value = 0)
        : value{initial_value}
    {
        ++alive;
    }

    counted_value(const counted_value& other)
        : value{other.value}
    {
        ++alive;
    }

    counted_value(counted_value&& other) noexcept
        : value{other.value}
    {
        other.value = -1;
        ++alive;
    }

    counted_value& operator=(const counted_value& other)
    {
        value = other.value;
        return *this;
    }

    counted_value& operator=(counted_value&& other) noexcept
    {
        value = other.value;
        other.value = -1;
        return *this;
    }

    ~counted_value()
    {
        --alive;
    }

    friend bool operator==(const counted_value&, const counted_value&) = default;
};

static void test_vector_against_std()
{
    hsd::vector<int> actual;
    std::vector<int> expected;

    for (int step = 0; step < 600; ++step)
    {
        switch (step % 9)
        {
            case 0:
            case 1:
                actual.push_back(step * 3 - 17);
                expected.push_back(step * 3 - 17);
                break;
            case 2:
                if (!expected.empty())
                {
                    actual.pop_back();
                    expected.pop_back();
                }
                break;
            case 3:
            {
                const auto new_size = static_cast<std::size_t>(step % 23);
                actual.resize(new_size);
                expected.resize(new_size);
                break;
            }
            case 4:
                actual.reserve(static_cast<std::size_t>(step + 1));
                expected.reserve(static_cast<std::size_t>(step + 1));
                break;
            case 5:
                if (!expected.empty())
                {
                    const auto index = static_cast<std::size_t>(step) % expected.size();
                    actual.erase(actual.begin() + index).unwrap();
                    expected.erase(expected.begin() + index);
                }
                break;
            case 6:
                actual.clear();
                expected.clear();
                break;
            case 7:
                actual.emplace_back(step * step);
                expected.emplace_back(step * step);
                break;
            case 8:
                if (!expected.empty())
                {
                    assert(actual.front() == expected.front());
                    assert(actual.back() == expected.back());
                    const auto index = static_cast<std::size_t>(step) % expected.size();
                    assert(actual[index] == expected[index]);
                }
                break;
        }

        test_support::expect_sequence(actual, expected);
    }

    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    test_support::expect_sequence(actual, expected);
}

static void test_vector_special_members()
{
    hsd::vector<int> source{{1, 2, 3, 5, 8, 13}};
    hsd::vector<int> copy{source};
    test_support::expect_sequence(copy, std::vector<int>{1, 2, 3, 5, 8, 13});

    source[0] = 99;
    assert(copy[0] == 1);

    hsd::vector<int> assigned;
    assigned = source;
    test_support::expect_sequence(assigned, std::vector<int>{99, 2, 3, 5, 8, 13});

    hsd::vector<int> moved{hsd::move(source)};
    test_support::expect_sequence(moved, std::vector<int>{99, 2, 3, 5, 8, 13});

    hsd::vector<int> move_assigned;
    move_assigned = hsd::move(moved);
    test_support::expect_sequence(move_assigned, std::vector<int>{99, 2, 3, 5, 8, 13});
}

static void test_vector_lifetime()
{
    assert(counted_value::alive == 0);

    {
        hsd::vector<counted_value> values;
        for (int index = 0; index < 64; ++index)
        {
            values.emplace_back(index);
        }

        assert(counted_value::alive == 64);
        values.erase(values.begin() + 7).unwrap();
        assert(counted_value::alive == 63);
        values.resize(11);
        assert(counted_value::alive == 11);
        values.clear();
        assert(counted_value::alive == 0);
    }

    assert(counted_value::alive == 0);
}

static void test_vector_edge_cases()
{
    hsd::vector<int> values;
    assert(values.size() == 0);
    assert(values.begin() == values.end());
    assert(!values.at(0).is_ok());

    values.pop_back();
    values.resize(0);
    values.reserve(32);
    assert(values.size() == 0);

    values.emplace_back(7);
    values.emplace_back(8);
    values.erase(values.begin()).unwrap();
    assert(values.size() == 1 && values.front() == 8);
    values.erase(values.begin()).unwrap();
    assert(values.size() == 0);

    values = {1, 2, 3};
    values = values;
    test_support::expect_sequence(values, std::vector<int>{1, 2, 3});

    hsd::static_vector<int, 2> fixed;
    fixed.emplace_back(1);
    fixed.emplace_back(2);
    assert(fixed.size() == 2);
}

int main()
{
    test_vector_against_std();
    test_vector_special_members();
    test_vector_lifetime();
    test_vector_edge_cases();

    constexpr auto values = []
    {
        hsd::static_vector<int, 4> result;
        result.emplace_back(1);
        result.emplace_back(2);
        result.emplace_back(3);
        result.emplace_back(4);
        return result;
    }();
    static_assert(values.size() == 4);
    assert(values.front() == 1);
    assert(values.back() == 4);
}
