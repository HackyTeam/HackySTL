// Title: Stack array at compile time
// Level: Advanced
// Shows: building lookup tables in constexpr functions and checking them with static_assert.

#include <Containers/StackArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// stack_array is a literal type, so a table can be computed during compilation.
constexpr hsd::stack_array<hsd::u64, 16> make_fibonacci()
{
    hsd::stack_array<hsd::u64, 16> table;
    table[0] = 0;
    table[1] = 1;
    for (hsd::usize idx = 2; idx < table.size(); ++idx)
    {
        table[idx] = table[idx - 1] + table[idx - 2];
    }
    return table;
}

constexpr hsd::stack_array<hsd::u32, 10> make_squares()
{
    hsd::stack_array<hsd::u32, 10> table;
    for (hsd::usize idx = 0; idx < table.size(); ++idx)
    {
        table[idx] = static_cast<hsd::u32>(idx * idx);
    }
    return table;
}

constexpr hsd::u32 sum_of(const hsd::stack_array<hsd::u32, 10>& table)
{
    hsd::u32 sum = 0;
    for (const auto& value : table)
    {
        sum += value;
    }
    return sum;
}

// fill() and swap() are constexpr as well.
constexpr hsd::u32 swapped_first()
{
    hsd::stack_array<hsd::u32, 4> lhs;
    hsd::stack_array<hsd::u32, 4> rhs;
    lhs.fill(3);
    rhs.fill(8);
    hsd::swap(lhs, rhs);
    return lhs[0] * 10 + rhs[0];
}

constexpr auto fibonacci = make_fibonacci();
constexpr auto squares = make_squares();

// These checks run while compiling: a wrong table would stop the build.
static_assert(fibonacci.size() == 16);
static_assert(fibonacci[10] == 55);
static_assert(fibonacci[15] == 610);
static_assert(squares[9] == 81);
static_assert(sum_of(squares) == 285);
static_assert(swapped_first() == 83);
static_assert(fibonacci.at(15).is_ok());
static_assert(!fibonacci.at(16).is_ok());

int main()
{
    hsd::print("fibonacci:"_fmt);
    for (const auto& value : fibonacci)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);

    hsd::print("squares:"_fmt);
    for (const auto& value : squares)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);

    hsd::println("sum of squares: {}"_fmt, sum_of(squares));
    hsd::println("swapped_first: {}"_fmt, swapped_first());

    return 0;
}
