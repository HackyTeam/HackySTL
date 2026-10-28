#include <Base/Algorithm.hpp>
#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <random>
#include <string>
#include <vector>

constexpr bool sorts_at_compile_time()
{
    std::array<int, 50> values{};

    for (int i = 0; i < 50; ++i)
    {
        values[i] = (i * 31 + 7) % 50;
    }

    hsd::quick_sort(values);

    for (int i = 1; i < 50; ++i)
    {
        if (values[i - 1] > values[i])
        {
            return false;
        }
    }

    return true;
}
static_assert(sorts_at_compile_time());

enum class input_shape
{
    random,
    ascending,
    descending,
    constant,
    two_values,
    few_values,
    organ_pipe,
    sawtooth,
    signed_random
};

constexpr input_shape all_shapes[] = {
    input_shape::random,
    input_shape::ascending,
    input_shape::descending,
    input_shape::constant,
    input_shape::two_values,
    input_shape::few_values,
    input_shape::organ_pipe,
    input_shape::sawtooth,
    input_shape::signed_random,
};

static std::vector<int> make_input(input_shape shape, std::size_t n, std::mt19937_64& rng)
{
    std::vector<int> values(n);

    for (std::size_t i = 0; i < n; ++i)
    {
        switch (shape)
        {
            case input_shape::random:
                values[i] = static_cast<int>(rng() % 1000000);
                break;

            case input_shape::ascending:
                values[i] = static_cast<int>(i);
                break;

            case input_shape::descending:
                values[i] = static_cast<int>(n - i);
                break;

            case input_shape::constant:
                values[i] = 42;
                break;

            case input_shape::two_values:
                values[i] = static_cast<int>(rng() % 2);
                break;

            case input_shape::few_values:
                values[i] = static_cast<int>(rng() % 7);
                break;

            case input_shape::organ_pipe:
                values[i] = static_cast<int>(i < n / 2 ? i : n - i);
                break;

            case input_shape::sawtooth:
                values[i] = static_cast<int>(i % 37);
                break;

            case input_shape::signed_random:
                values[i] = static_cast<int>(rng() % 2000) - 1000;
                break;
        }
    }

    return values;
}

struct record
{
    int key;
    int id;

    friend bool operator>(const record& lhs, const record& rhs)
    {
        return lhs.key > rhs.key;
    }
};

int main()
{
    std::mt19937_64 rng(777);

    std::vector<std::size_t> sizes;

    for (std::size_t n = 0; n <= 40; ++n)
    {
        sizes.push_back(n);
    }

    for (std::size_t n : {63u, 64u, 65u, 127u, 128u, 129u, 255u, 256u, 257u, 1000u, 4097u, 8000u})
    {
        sizes.push_back(n);
    }

    for (input_shape shape : all_shapes)
    {
        for (std::size_t n : sizes)
        {
            const auto input = make_input(shape, n, rng);
            auto expected = input;
            std::sort(expected.begin(), expected.end());

            // hsd::vector, ascending.
            hsd::vector<int> own;

            for (int value : input)
            {
                own.push_back(value);
            }

            hsd::quick_sort(own);

            for (std::size_t i = 0; i < n; ++i)
            {
                assert(own[i] == expected[i]);
            }

            // A different container type, ascending.
            auto plain = input;
            hsd::quick_sort(plain);
            assert(plain == expected);

            // Predicate: a goes after b when a < b, so the order is descending.
            auto descending = input;
            hsd::quick_sort(descending, [](int a, int b) { return a < b; });

            auto expected_descending = input;
            std::sort(expected_descending.begin(), expected_descending.end(), std::greater<int>());
            assert(descending == expected_descending);

            // Records: ordered by key, with nothing lost or duplicated.
            std::vector<record> records(n);

            for (std::size_t i = 0; i < n; ++i)
            {
                records[i] = {input[i], static_cast<int>(i)};
            }

            hsd::quick_sort(records);

            for (std::size_t i = 1; i < n; ++i)
            {
                assert(records[i - 1].key <= records[i].key);
            }

            std::vector<int> ids;

            for (const auto& item : records)
            {
                ids.push_back(item.id);
            }

            std::sort(ids.begin(), ids.end());

            for (std::size_t i = 0; i < n; ++i)
            {
                assert(ids[i] == static_cast<int>(i));
            }
        }
    }

    // Owning elements: nothing leaked, double destroyed, or used after a move.
    for (std::size_t n : {0u, 1u, 2u, 17u, 18u, 100u, 2000u})
    {
        for (input_shape shape : all_shapes)
        {
            const auto input = make_input(shape, n, rng);
            std::vector<std::string> expected;
            hsd::vector<hsd::string> own;

            for (int value : input)
            {
                const std::string text = std::string(30, 'a') + std::to_string(value + 100000);
                expected.push_back(text);
                own.push_back(hsd::string(text.c_str()));
            }

            std::sort(expected.begin(), expected.end());
            hsd::quick_sort(own);

            for (std::size_t i = 0; i < n; ++i)
            {
                assert(std::string(own[i].c_str()) == expected[i]);
            }
        }
    }

    // Sorted, reversed and constant inputs of this size take quadratic time or
    // overflow the stack with a naive first-element pivot.
    const input_shape big_shapes[] = {
        input_shape::ascending,
        input_shape::descending,
        input_shape::constant,
        input_shape::organ_pipe,
        input_shape::two_values,
    };

    for (input_shape shape : big_shapes)
    {
        auto big = make_input(shape, 100000, rng);
        auto expected = big;
        std::sort(expected.begin(), expected.end());
        hsd::quick_sort(big);
        assert(big == expected);
    }

    return 0;
}
