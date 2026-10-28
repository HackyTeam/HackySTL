#include <Base/Algorithm.hpp>
#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <random>
#include <string>
#include <vector>

constexpr bool test_sub_ranges_at_compile_time()
{
    std::array<int, 6> insertion{5, 4, 3, 2, 1, 0};
    hsd::insertion_sorter<std::array<int, 6>>::sort(insertion, 1, 4);

    std::array<int, 6> heap{5, 4, 3, 2, 1, 0};
    hsd::heap_sorter<std::array<int, 6>>::sort(heap, 1, 4);

    const std::array<int, 6> expected{5, 1, 2, 3, 4, 0};
    return insertion == expected && heap == expected;
}
static_assert(test_sub_ranges_at_compile_time());

constexpr bool test_free_sorts_at_compile_time()
{
    std::array<int, 50> values{};

    for (int i = 0; i < 50; ++i)
    {
        values[i] = (i * 31 + 7) % 50;
    }

    auto insertion = values;
    hsd::insertion_sort(insertion);

    auto heap = values;
    hsd::heap_sort(heap);

    auto intro = values;
    hsd::introsort(intro);

    for (int i = 0; i < 50; ++i)
    {
        if (insertion[i] != i || heap[i] != i || intro[i] != i)
        {
            return false;
        }
    }

    return true;
}
static_assert(test_free_sorts_at_compile_time());

static std::vector<int> random_values(std::size_t n, std::mt19937& rng)
{
    std::vector<int> values(n);

    // Few distinct values, so duplicates are common.
    for (auto& value : values)
    {
        value = static_cast<int>(rng() % 20) - 10;
    }

    return values;
}

template <typename Vec>
Vec make_vec(const std::vector<int>& values)
{
    Vec out;

    for (int value : values)
    {
        out.push_back(value);
    }

    return out;
}

// Checks that the leading elements of `actual` match `expected`.
template <typename Vec>
void check_elements(const Vec& actual, const std::vector<int>& expected)
{
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        assert(actual[i] == expected[i]);
    }
}

// Sorts a random inclusive sub-range, then checks that the elements outside it
// did not move and that the range now holds the sorted values.
template <typename Vec, template <typename> class Sorter>
void test_range_sorter(std::mt19937& rng, std::size_t max_n)
{
    for (std::size_t n = 1; n <= max_n; ++n)
    {
        for (int trial = 0; trial < 6; ++trial)
        {
            const std::vector<int> values = random_values(n, rng);
            const std::size_t low = rng() % n;
            const std::size_t high = low + rng() % (n - low);

            for (bool descending : {false, true})
            {
                Vec data = make_vec<Vec>(values);

                if (descending)
                {
                    auto pred = std::less<>();
                    Sorter<Vec>::sort(data, pred, low, high);
                }
                else
                {
                    Sorter<Vec>::sort(data, low, high);
                }

                for (std::size_t i = 0; i < low; ++i)
                {
                    assert(data[i] == values[i]);
                }

                for (std::size_t i = high + 1; i < n; ++i)
                {
                    assert(data[i] == values[i]);
                }

                std::vector<int> expected(values.begin() + low, values.begin() + high + 1);

                if (descending)
                {
                    std::sort(expected.begin(), expected.end(), std::greater<int>());
                }
                else
                {
                    std::sort(expected.begin(), expected.end());
                }

                for (std::size_t i = 0; i < expected.size(); ++i)
                {
                    assert(data[low + i] == expected[i]);
                }
            }
        }
    }
}

// After partitioning, the pivot sits at the returned index with nothing before it
// that must come after it and nothing after it that must come before it.
template <typename Vec>
void test_partition(std::mt19937& rng)
{
    auto greater = std::greater<>();

    for (std::size_t n = 2; n <= 60; ++n)
    {
        for (int trial = 0; trial < 6; ++trial)
        {
            const std::vector<int> values = random_values(n, rng);
            const std::size_t low = rng() % (n - 1);
            const std::size_t high = low + 1 + rng() % (n - low - 1);

            for (bool with_pred : {false, true})
            {
                Vec data = make_vec<Vec>(values);

                const std::size_t pivot = with_pred
                    ? hsd::quick_sorter<Vec>::partition(data, greater, low, high)
                    : hsd::quick_sorter<Vec>::partition(data, low, high);

                assert(low <= pivot && pivot <= high);

                for (std::size_t i = low; i < pivot; ++i)
                {
                    assert(!(data[i] > data[pivot]));
                }

                for (std::size_t i = pivot + 1; i <= high; ++i)
                {
                    assert(!(data[pivot] > data[i]));
                }

                // Partitioning only permutes the range.
                std::vector<int> before(values.begin() + low, values.begin() + high + 1);
                std::vector<int> after;

                for (std::size_t i = low; i <= high; ++i)
                {
                    after.push_back(data[i]);
                }

                std::sort(before.begin(), before.end());
                std::sort(after.begin(), after.end());
                assert(before == after);

                for (std::size_t i = 0; i < low; ++i)
                {
                    assert(data[i] == values[i]);
                }

                for (std::size_t i = high + 1; i < n; ++i)
                {
                    assert(data[i] == values[i]);
                }
            }
        }
    }
}

// The free functions sort the whole container, and each one agrees with std::sort
// in both orders.
template <typename Vec>
void test_free_sorts(std::mt19937& rng)
{
    auto less = std::less<>();

    for (std::size_t n = 0; n <= 300; ++n)
    {
        for (int trial = 0; trial < 3; ++trial)
        {
            const std::vector<int> values = random_values(n, rng);

            auto ascending = values;
            std::sort(ascending.begin(), ascending.end());

            auto descending = values;
            std::sort(descending.begin(), descending.end(), std::greater<int>());

            Vec insertion = make_vec<Vec>(values);
            hsd::insertion_sort(insertion);
            check_elements(insertion, ascending);

            Vec heap = make_vec<Vec>(values);
            hsd::heap_sort(heap);
            check_elements(heap, ascending);

            Vec intro = make_vec<Vec>(values);
            hsd::introsort(intro);
            check_elements(intro, ascending);

            Vec quick = make_vec<Vec>(values);
            hsd::quick_sort(quick);
            check_elements(quick, ascending);

            Vec insertion_desc = make_vec<Vec>(values);
            hsd::insertion_sort(insertion_desc, less);
            check_elements(insertion_desc, descending);

            Vec heap_desc = make_vec<Vec>(values);
            hsd::heap_sort(heap_desc, less);
            check_elements(heap_desc, descending);

            Vec intro_desc = make_vec<Vec>(values);
            hsd::introsort(intro_desc, less);
            check_elements(intro_desc, descending);
        }
    }
}

// The free functions sort the characters of an hsd::string and leave the
// terminator where it was.
static void test_string_sorts()
{
    const std::string text = "the quick brown fox jumps over the lazy dog";
    std::string expected = text;
    std::sort(expected.begin(), expected.end());

    hsd::string insertion(text.c_str());
    hsd::insertion_sort(insertion);
    assert(std::string(insertion.c_str()) == expected);

    hsd::string heap(text.c_str());
    hsd::heap_sort(heap);
    assert(std::string(heap.c_str()) == expected);

    hsd::string intro(text.c_str());
    hsd::introsort(intro);
    assert(std::string(intro.c_str()) == expected);
}

// std::string elements are moved by the sorters, so they must resolve without ADL
// ambiguity like any other element type.
static void test_std_string_elements()
{
    hsd::vector<std::string> values;

    for (const char* text : {"pear", "apple", "fig", "date", "cherry", "banana"})
    {
        values.push_back(std::string(text));
    }

    hsd::insertion_sort(values);
    assert(values[0] == "apple" && values[5] == "pear");

    hsd::heap_sort(values);
    assert(values[0] == "apple" && values[5] == "pear");

    hsd::introsort(values);
    assert(values[0] == "apple" && values[5] == "pear");
}

int main()
{
    std::mt19937 rng(31337);

    test_range_sorter<std::vector<int>, hsd::insertion_sorter>(rng, 70);
    test_range_sorter<std::vector<int>, hsd::heap_sorter>(rng, 70);
    test_range_sorter<std::vector<int>, hsd::introsorter>(rng, 600);

    test_range_sorter<hsd::vector<int>, hsd::insertion_sorter>(rng, 70);
    test_range_sorter<hsd::vector<int>, hsd::heap_sorter>(rng, 70);
    test_range_sorter<hsd::vector<int>, hsd::introsorter>(rng, 600);

    test_partition<std::vector<int>>(rng);
    test_partition<hsd::vector<int>>(rng);

    test_free_sorts<std::vector<int>>(rng);
    test_free_sorts<hsd::vector<int>>(rng);
    test_string_sorts();
    test_std_string_elements();
    return 0;
}
