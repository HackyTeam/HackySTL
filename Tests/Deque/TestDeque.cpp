#include <cassert>
#include <deque>
#include <print>

#include <Containers/Deque.hpp>
#include <Support/TestSupport.hpp>

static void test_deque_against_std()
{
    hsd::deque<int> actual;
    std::deque<int> expected;

    for (int step = 0; step < 700; ++step)
    {
        switch (step % 10)
        {
            case 0:
            case 1:
                actual.push_back(step);
                expected.push_back(step);
                break;
            case 2:
                actual.push_front(-step);
                expected.push_front(-step);
                break;
            case 3:
                if (!expected.empty())
                {
                    actual.pop_back();
                    expected.pop_back();
                }
                break;
            case 4:
                if (!expected.empty())
                {
                    actual.pop_front();
                    expected.pop_front();
                }
                break;
            case 5:
                actual.resize(static_cast<std::size_t>(step % 29));
                expected.resize(static_cast<std::size_t>(step % 29));
                break;
            case 6:
                if (!expected.empty())
                {
                    const auto index = static_cast<std::size_t>(step) % expected.size();
                    auto actual_iter = actual.begin();
                    for (std::size_t current = 0; current < index; ++current)
                    {
                        ++actual_iter;
                    }
                    actual.erase(actual_iter).unwrap();
                    expected.erase(expected.begin() + index);
                }
                break;
            case 7:
                actual.emplace_back(step * 2);
                expected.emplace_back(step * 2);
                break;
            case 8:
                actual.emplace_front(step * -2);
                expected.emplace_front(step * -2);
                break;
            case 9:
                if (!expected.empty())
                {
                    assert(actual.front() == expected.front());
                    assert(actual.back() == expected.back());
                }
                break;
        }

        if (actual.size() != expected.size())
        {
            std::println(
                stderr,
                "deque size mismatch after step {}: actual={} expected={}",
                step, actual.size(), expected.size()
            );
        }
        if (!expected.empty() && actual.front() != expected.front())
        {
            std::println(
                stderr,
                "deque front mismatch after step {}: actual={} expected={}",
                step, actual.front(), expected.front()
            );
        }
        test_support::expect_sequence(actual, expected);
        assert(actual.size() == expected.size());
    }
}

static void test_deque_special_members()
{
    hsd::deque<int> source{{1, 2, 3, 4}};
    hsd::deque<int> copy{source};
    test_support::expect_sequence(copy, std::deque<int>{1, 2, 3, 4});

    hsd::deque<int> assigned;
    assigned = source;
    test_support::expect_sequence(assigned, std::deque<int>{1, 2, 3, 4});

    hsd::deque<int> moved{hsd::move(source)};
    test_support::expect_sequence(moved, std::deque<int>{1, 2, 3, 4});

    hsd::deque<int> move_assigned;
    move_assigned = hsd::move(moved);
    test_support::expect_sequence(move_assigned, std::deque<int>{1, 2, 3, 4});

    move_assigned.clear();
    assert(move_assigned.size() == 0);
}

static void test_deque_edge_cases()
{
    hsd::deque<int> values;
    assert(values.size() == 0);
    values.pop_front();
    values.pop_back();
    values.resize(0);
    assert(values.begin() == values.end());

    for (int value = 0; value < 32; ++value)
    {
        values.push_back(value);
    }
    for (int value = 0; value < 16; ++value)
    {
        values.pop_front();
    }
    values.resize(20);
    assert(values.front() == 16);
    assert(values.back() == 0);

    auto iter = values.begin();
    values.erase(iter).unwrap();
    auto last = values.end();
    values.erase(--last).unwrap();
    assert(values.front() == 17);

    values.reserve(64);
    assert(values.capacity() >= 64);
    test_support::expect_sequence(
        values,
        std::deque<int>{
            17, 18, 19, 20, 21, 22, 23, 24, 25,
            26, 27, 28, 29, 30, 31, 0, 0, 0
        }
    );
}

int main()
{
    test_deque_against_std();
    test_deque_special_members();
    test_deque_edge_cases();
}
