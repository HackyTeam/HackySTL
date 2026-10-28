#include <Base/Algorithm.hpp>
#include <Containers/Vector.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

constexpr bool test_sorts_at_compile_time()
{
    std::array<unsigned, 50> values{};

    for (unsigned i = 0; i < 50; ++i)
    {
        values[i] = (i * 31 + 7) % 50;
    }

    hsd::radix_sort(values);

    for (unsigned i = 1; i < 50; ++i)
    {
        if (values[i - 1] > values[i])
        {
            return false;
        }
    }

    return true;
}
static_assert(test_sorts_at_compile_time());

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
    near_max,
    straddle
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
    input_shape::near_max,
    input_shape::straddle,
};

static std::vector<std::uint64_t> make_input(input_shape shape, std::size_t n, std::mt19937_64& rng)
{
    std::vector<std::uint64_t> keys(n);

    for (std::size_t i = 0; i < n; ++i)
    {
        switch (shape)
        {
            case input_shape::random:
                keys[i] = rng();
                break;

            case input_shape::ascending:
                keys[i] = i;
                break;

            case input_shape::descending:
                keys[i] = n - i;
                break;

            case input_shape::constant:
                keys[i] = 42;
                break;

            case input_shape::two_values:
                keys[i] = rng() % 2;
                break;

            case input_shape::few_values:
                keys[i] = rng() % 7;
                break;

            case input_shape::organ_pipe:
                keys[i] = i < n / 2 ? i : n - i;
                break;

            case input_shape::sawtooth:
                keys[i] = i % 37;
                break;

            // Largest 64-bit values: the top place value is 10^19.
            case input_shape::near_max:
                keys[i] = UINT64_MAX - rng() % 1000;
                break;

            // Keys straddling 10^19, where the place value is widest.
            case input_shape::straddle:
                keys[i] = 10000000000000000000ull - 500 + rng() % 1000;
                break;
        }
    }

    return keys;
}

struct record
{
    std::uint64_t key;
    std::uint64_t id;
};

static void test_records_by_key(const std::vector<std::uint64_t>& input)
{
    const std::size_t n = input.size();
    std::vector<record> records(n);

    for (std::size_t i = 0; i < n; ++i)
    {
        records[i] = {input[i], i};
    }

    // Ordered by key. Equal keys keep their input order because the sort is stable.
    hsd::radix_sort(
        records,
        [](const record& lhs, const record& rhs) { return lhs.key > rhs.key; },
        [](const record& item) { return item.key; });

    for (std::size_t i = 1; i < n; ++i)
    {
        assert(records[i - 1].key <= records[i].key);

        if (records[i - 1].key == records[i].key)
        {
            assert(records[i - 1].id < records[i].id);
        }
    }

    std::vector<std::uint64_t> ids;

    for (const auto& item : records)
    {
        ids.push_back(item.id);
    }

    std::sort(ids.begin(), ids.end());

    for (std::size_t i = 0; i < n; ++i)
    {
        assert(ids[i] == i);
    }
}

static void test_shapes(std::mt19937_64& rng)
{
    std::vector<std::size_t> sizes;

    for (std::size_t n = 0; n <= 40; ++n)
    {
        sizes.push_back(n);
    }

    for (std::size_t n : {63u, 64u, 65u, 127u, 128u, 129u, 255u, 256u, 257u, 1000u, 4097u})
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
            hsd::vector<std::uint64_t> own;

            for (std::uint64_t value : input)
            {
                own.push_back(value);
            }

            hsd::radix_sort(own);

            for (std::size_t i = 0; i < n; ++i)
            {
                assert(own[i] == expected[i]);
            }

            // A different container type, ascending.
            auto plain = input;
            hsd::radix_sort(plain);
            assert(plain == expected);

            // The predicate orders keys the way operator> does, but the passes always
            // produce ascending keys, so ascending is the only valid order.
            auto ordered_by_pred = input;
            hsd::radix_sort(ordered_by_pred, std::greater<>());
            assert(ordered_by_pred == expected);

            test_records_by_key(input);
        }
    }
}

static void test_large_keys(std::mt19937_64& rng)
{
    // Sorted, reversed and near-maximal inputs exercise every place value. The
    // largest keys used to divide by zero once the place value wrapped around.
    const input_shape shapes[] = {
        input_shape::ascending,
        input_shape::descending,
        input_shape::constant,
        input_shape::near_max,
        input_shape::straddle,
        input_shape::random,
    };

    for (input_shape shape : shapes)
    {
        auto big = make_input(shape, 100000, rng);
        auto expected = big;
        std::sort(expected.begin(), expected.end());

        hsd::radix_sort(big);
        assert(big == expected);
    }
}

static void test_count_sorter_pass()
{
    // One counting pass orders by the units digit and keeps equal digits in their
    // input order: the units digits are 3, 1, 2, 0, 3 for the values below.
    hsd::vector<unsigned> digits;

    for (unsigned value : {13u, 21u, 12u, 30u, 3u})
    {
        digits.push_back(value);
    }

    hsd::count_sorter<hsd::vector<unsigned>>::sort(digits, 1u);
    assert(digits[0] == 30 && digits[1] == 21 && digits[2] == 12);
    assert(digits[3] == 13 && digits[4] == 3);
}

int main()
{
    std::mt19937_64 rng(4242);

    test_shapes(rng);
    test_large_keys(rng);
    test_count_sorter_pass();
    return 0;
}
