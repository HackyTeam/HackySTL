// Title: Compile-time tables from sequences
// Level: Advanced
// Shows: building constexpr lookup tables from index sequences, checked with static_assert.

#include <Base/IntegerSequence.hpp>
#include <Containers/StackArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

constexpr hsd::u64 factorial(hsd::usize n)
{
    hsd::u64 result = 1;

    for (hsd::usize factor = 2; factor <= n; factor++)
    {
        result *= factor;
    }

    return result;
}

constexpr hsd::u64 fibonacci(hsd::usize n)
{
    hsd::u64 previous = 0;
    hsd::u64 current = 1;

    for (hsd::usize step = 0; step < n; step++)
    {
        hsd::u64 next = previous + current;
        previous = current;
        current = next;
    }

    return previous;
}

// The index pack runs the function once per slot, so the whole table is
// computed while the program is compiled.
template <typename Function, hsd::usize... indices>
constexpr auto make_table(Function function, hsd::index_sequence<indices...>)
{
    return hsd::stack_array<hsd::u64, sizeof...(indices)>{{function(indices)...}};
}

template <hsd::usize N, typename Function>
constexpr auto make_table(Function function)
{
    return make_table(function, hsd::make_index_sequence<N>{});
}

// The same pack can walk a table from the end to the start.
template <hsd::usize N, hsd::usize... indices>
constexpr hsd::u64 reversed_weighted_sum(
    const hsd::stack_array<hsd::u64, N>& table, hsd::index_sequence<indices...>)
{
    hsd::u64 total = 0;
    hsd::usize weight = 1;
    ((total += table[indices] * weight++), ...);
    return total;
}

constexpr auto factorials = make_table<10>(factorial);
constexpr auto fibonacci_numbers = make_table<12>(fibonacci);

static_assert(factorials[5] == 120);
static_assert(factorials[9] == 362880);
static_assert(fibonacci_numbers[10] == 55);
static_assert(fibonacci_numbers[11] == 89);

// Reads entries 2, 1, 0 with weights 1, 2, 3: 2! * 1 + 1! * 2 + 0! * 3.
constexpr hsd::u64 weighted = reversed_weighted_sum(
    factorials, hsd::inverse::ranged::make_index_sequence<3, 0>{}
);
static_assert(weighted == 2 * 1 + 1 * 2 + 1 * 3);

template <hsd::usize N>
void print_table(const char* label, const hsd::stack_array<hsd::u64, N>& table)
{
    hsd::print("{}:"_fmt, label);
    for (hsd::usize index = 0; index < N; index++)
    {
        hsd::print(" {}"_fmt, table[index]);
    }
    hsd::println(""_fmt);
}

int main()
{
    print_table("factorials", factorials);
    print_table("fibonacci", fibonacci_numbers);
    hsd::println("weighted sum: {}"_fmt, weighted);

    return 0;
}
