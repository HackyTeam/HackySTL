#include <cassert>
#include <unordered_map>

#include <Containers/UnorderedMap.hpp>
#include <Support/TestSupport.hpp>

static void erase_key(hsd::unordered_map<int, int>& values, int key)
{
    for (auto iter = values.begin(); iter != values.end(); ++iter)
    {
        if (iter->first == key)
        {
            values.erase(iter).unwrap();
            return;
        }
    }

    assert(false);
}

static void test_map_against_std()
{
    hsd::unordered_map<int, int> actual;
    std::unordered_map<int, int> expected;

    for (int step = 0; step < 900; ++step)
    {
        const int key = (step * 37) % 113;
        switch (step % 7)
        {
            case 0:
            case 1:
            {
                const int value = step * step - 3 * step;
                auto actual_result = actual.emplace(key, value);
                auto expected_result = expected.emplace(key, value);
                assert(actual_result.second == expected_result.second);
                break;
            }
            case 2:
                actual[key] = step - 100;
                expected[key] = step - 100;
                break;
            case 3:
                if (expected.contains(key))
                {
                    erase_key(actual, key);
                    expected.erase(key);
                }
                break;
            case 4:
                if (expected.contains(key))
                {
                    auto value = actual.at(key);
                    assert(value.is_ok());
                    assert(value.unwrap().get() == expected.at(key));
                }
                else
                {
                    assert(!actual.at(key).is_ok());
                }
                break;
            case 5:
                if (step % 31 == 0)
                {
                    actual.clear();
                    expected.clear();
                }
                break;
            case 6:
                actual.emplace(key + 1000, step);
                expected.emplace(key + 1000, step);
                break;
        }

        test_support::expect_map(actual, expected);
    }
}

static void test_map_special_members()
{
    hsd::unordered_map<int, int> source;
    source.emplace(1, 11);
    source.emplace(2, 22);
    source.emplace(3, 33);

    std::unordered_map<int, int> expected{{1, 11}, {2, 22}, {3, 33}};
    hsd::unordered_map<int, int> copy{source};
    test_support::expect_map(copy, expected);

    source[1] = 100;
    assert(copy.at(1).unwrap().get() == 11);

    hsd::unordered_map<int, int> assigned;
    assigned = source;
    test_support::expect_map(
        assigned, std::unordered_map<int, int>{{1, 100}, {2, 22}, {3, 33}}
    );

    hsd::unordered_map<int, int> moved{hsd::move(source)};
    test_support::expect_map(
        moved, std::unordered_map<int, int>{{1, 100}, {2, 22}, {3, 33}}
    );
    moved.clear();
    assert(moved.size() == 0);
}

static void test_map_edge_cases()
{
    hsd::unordered_map<int, int> values;
    assert(values.size() == 0);
    assert(!values.at(99).is_ok());

    auto first = values.emplace(1, 10);
    auto duplicate = values.emplace(1, 20);
    assert(first.second);
    assert(!duplicate.second);
    assert(values.at(1).unwrap().get() == 10);

    values[1] = 11;
    assert(values[1] == 11);
    values.clear();
    assert(values.size() == 0);
}

static void test_map_reserve()
{
    hsd::unordered_map<int, int> actual;
    actual.emplace(-1, 17);
    actual.reserve(1000);
    assert(actual.at(-1).unwrap().get() == 17);

    std::unordered_map<int, int> expected{{-1, 17}};
    expected.reserve(1000);

    for (int key = 0; key < 1000; ++key)
    {
        actual.emplace(key, key * 2);
        expected.emplace(key, key * 2);
    }

    test_support::expect_map(actual, expected);
}

int main()
{
    test_map_against_std();
    test_map_special_members();
    test_map_edge_cases();
    test_map_reserve();
}
