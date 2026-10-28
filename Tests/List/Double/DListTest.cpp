#include <cassert>
#include <list>

#include <Containers/List.hpp>
#include <Support/TestSupport.hpp>

static void erase_at(hsd::list<int>& values, std::size_t index)
{
    auto iter = values.begin();
    for (std::size_t current = 0; current < index; ++current)
    {
        ++iter;
    }
    values.erase(iter).unwrap();
}

static void test_list_against_std()
{
    hsd::list<int> actual;
    std::list<int> expected;

    for (int step = 0; step < 500; ++step)
    {
        switch (step % 8)
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
                if (!expected.empty())
                {
                    const auto index = static_cast<std::size_t>(step) % expected.size();
                    erase_at(actual, index);
                    auto iter = expected.begin();
                    std::advance(iter, static_cast<long>(index));
                    expected.erase(iter);
                }
                break;
            case 6:
                actual.emplace_back(step * step);
                expected.emplace_back(step * step);
                break;
            case 7:
                actual.emplace_front(step * -step);
                expected.emplace_front(step * -step);
                break;
        }

        test_support::expect_sequence(actual, expected);
        assert(actual.size() == expected.size());
        assert(actual.empty() == expected.empty());
        if (!expected.empty())
        {
            assert(actual.front() == expected.front());
            assert(actual.back() == expected.back());
        }
    }
}

static void test_list_special_members()
{
    hsd::list<int> source{{1, 2, 3, 4}};
    hsd::list<int> copy{source};
    test_support::expect_sequence(copy, std::list<int>{1, 2, 3, 4});

    hsd::list<int> assigned;
    assigned = source;
    test_support::expect_sequence(assigned, std::list<int>{1, 2, 3, 4});

    hsd::list<int> moved{hsd::move(source)};
    test_support::expect_sequence(moved, std::list<int>{1, 2, 3, 4});
    moved.clear();
    assert(moved.empty());
}

static void test_list_edge_cases()
{
    hsd::list<int> values;
    assert(values.empty());
    values.pop_front();
    values.pop_back();

    values.push_back(1);
    values.push_front(0);
    values.push_back(2);
    assert(values.front() == 0 && values.back() == 2);

    values.erase(values.begin()).unwrap();
    values.erase(values.rbegin()).unwrap();
    assert(values.size() == 1 && values.front() == 1);
    values.clear();
    assert(values.empty());
}

int main()
{
    test_list_against_std();
    test_list_special_members();
    test_list_edge_cases();
}
