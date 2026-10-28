#include <cassert>
#include <forward_list>
#include <iterator>

#include <Containers/ForwardList.hpp>
#include <Support/TestSupport.hpp>

static std::size_t list_size(const std::forward_list<int>& values)
{
    return static_cast<std::size_t>(
        std::distance(values.begin(), values.end())
    );
}

static void append(std::forward_list<int>& values, int value)
{
    auto iter = values.before_begin();
    while (std::next(iter) != values.end())
    {
        ++iter;
    }
    values.insert_after(iter, value);
}

static int back(const std::forward_list<int>& values)
{
    assert(!values.empty());
    auto iter = values.begin();
    auto last = iter;
    for (; iter != values.end(); ++iter)
    {
        last = iter;
    }
    return *last;
}

static void erase_at(
    hsd::forward_list<int>& values, std::size_t index)
{
    auto iter = values.begin();
    for (std::size_t current = 0; current < index; ++current)
    {
        ++iter;
    }
    values.erase(iter).unwrap();
}

static void test_forward_list_against_std()
{
    hsd::forward_list<int> actual;
    std::forward_list<int> expected;

    for (int step = 0; step < 500; ++step)
    {
        const auto expected_size = list_size(expected);
        switch (step % 7)
        {
            case 0:
            case 1:
                actual.push_back(step);
                append(expected, step);
                break;
            case 2:
                actual.push_front(-step);
                expected.push_front(-step);
                break;
            case 3:
                if (!expected.empty())
                {
                    actual.pop_front();
                    expected.pop_front();
                }
                break;
            case 4:
                if (expected_size != 0)
                {
                    const auto index = static_cast<std::size_t>(step) % expected_size;
                    erase_at(actual, index);
                    auto previous = expected.before_begin();
                    auto iter = expected.begin();
                    for (std::size_t current = 0; current < index; ++current)
                    {
                        ++previous;
                        ++iter;
                    }
                    expected.erase_after(previous);
                }
                break;
            case 5:
                actual.emplace_back(step * step);
                append(expected, step * step);
                break;
            case 6:
                actual.emplace_front(step * -step);
                expected.emplace_front(step * -step);
                break;
        }

        test_support::expect_sequence(actual, expected);
        assert(actual.size() == list_size(expected));
        assert(actual.empty() == expected.empty());
        if (!expected.empty())
        {
            assert(actual.front() == expected.front());
            assert(actual.back() == back(expected));
        }
    }
}

static void test_forward_list_special_members()
{
    hsd::forward_list<int> source{{1, 2, 3, 4}};
    hsd::forward_list<int> copy{source};
    test_support::expect_sequence(copy, std::forward_list<int>{1, 2, 3, 4});

    hsd::forward_list<int> assigned;
    assigned = source;
    test_support::expect_sequence(assigned, std::forward_list<int>{1, 2, 3, 4});

    hsd::forward_list<int> moved{hsd::move(source)};
    test_support::expect_sequence(moved, std::forward_list<int>{1, 2, 3, 4});
    moved.clear();
    assert(moved.empty());
}

static void test_forward_list_edge_cases()
{
    hsd::forward_list<int> values;
    assert(values.empty());
    values.pop_front();

    values.push_front(2);
    values.push_front(1);
    values.push_back(3);
    assert(values.front() == 1 && values.back() == 3);

    values.erase(values.begin()).unwrap();
    assert(values.front() == 2);
    values.clear();
    assert(values.empty());
}

int main()
{
    test_forward_list_against_std();
    test_forward_list_special_members();
    test_forward_list_edge_cases();
}
