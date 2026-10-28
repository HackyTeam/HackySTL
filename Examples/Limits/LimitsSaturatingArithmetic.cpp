// Title: Saturating arithmetic
// Level: Advanced
// Shows: generic saturating add, subtract and multiply built on limits, used for pixels and health bars.

#include <Base/Limits.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Saturating operations clamp to the nearest limit instead of wrapping.
// The work is done in i64 or f64 headroom where a wider type exists.
template <typename T>
constexpr T clamp_to(hsd::i64 wide)
{
    if (wide > static_cast<hsd::i64>(hsd::limits<T>::max))
    {
        return hsd::limits<T>::max;
    }
    if (wide < static_cast<hsd::i64>(hsd::limits<T>::min))
    {
        return hsd::limits<T>::min;
    }
    return static_cast<T>(wide);
}

template <typename T>
constexpr T sat_add(T left, T right)
{
    return clamp_to<T>(static_cast<hsd::i64>(left) + static_cast<hsd::i64>(right));
}

template <typename T>
constexpr T sat_sub(T left, T right)
{
    return clamp_to<T>(static_cast<hsd::i64>(left) - static_cast<hsd::i64>(right));
}

template <typename T>
constexpr T sat_mul(T left, T right)
{
    return clamp_to<T>(static_cast<hsd::i64>(left) * static_cast<hsd::i64>(right));
}

static_assert(sat_add<hsd::u8>(200, 100) == 255);
static_assert(sat_add<hsd::u8>(20, 100) == 120);
static_assert(sat_sub<hsd::u8>(10, 20) == 0);
static_assert(sat_add<hsd::i8>(-100, -100) == -128);
static_assert(sat_add<hsd::i8>(100, 100) == 127);
static_assert(sat_mul<hsd::i16>(300, 300) == 32767);
static_assert(sat_mul<hsd::i16>(-300, 300) == -32768);

// A pixel row stored as u8 where brightness changes must not wrap around.
static void brighten(hsd::vector<hsd::u8>& row, hsd::i16 amount)
{
    for (hsd::usize index = 0; index < row.size(); ++index)
    {
        hsd::i64 moved = static_cast<hsd::i64>(row[index]) + amount;
        row[index] = clamp_to<hsd::u8>(moved);
    }
}

static void print_row(const char* label, const hsd::vector<hsd::u8>& row)
{
    hsd::print("{}:"_fmt, label);
    for (hsd::usize index = 0; index < row.size(); ++index)
    {
        hsd::print(" {}"_fmt, static_cast<hsd::i32>(row[index]));
    }
    hsd::println(""_fmt);
}

// A health value in i16 whose healing and damage stop at the ends of the range.
struct health_bar
{
    hsd::i16 value;

    void hurt(hsd::i16 damage)
    {
        value = sat_sub<hsd::i16>(value, damage);
    }

    void heal(hsd::i16 amount)
    {
        value = sat_add<hsd::i16>(value, amount);
    }
};

int main()
{
    hsd::vector<hsd::u8> row{{0, 40, 128, 200, 255}};
    print_row("original", row);

    brighten(row, 70);
    print_row("brighter", row);

    brighten(row, -200);
    print_row("darker", row);

    // The naive u8 version wraps instead of clamping.
    hsd::u8 naive = 200;
    naive = static_cast<hsd::u8>(naive + 100);
    hsd::println("naive u8 200 + 100 = {}, saturating = {}"_fmt,
        static_cast<hsd::i32>(naive), static_cast<hsd::i32>(sat_add<hsd::u8>(200, 100)));

    health_bar hero{30000};
    hero.heal(5000);
    hsd::println("healed past the top: {}"_fmt, hero.value);
    hero.hurt(32000);
    hsd::println("after damage: {}"_fmt, hero.value);
    hero.hurt(32000);
    hsd::println("after more damage: {}"_fmt, hero.value);
    hero.hurt(32000);
    hsd::println("stops at the bottom: {}"_fmt, hero.value);

    // A running total in u16 that stops at the top instead of rolling over.
    hsd::u16 total = 0;
    hsd::u32 steps = 0;
    while (total != hsd::limits<hsd::u16>::max)
    {
        total = sat_add<hsd::u16>(total, 7000);
        ++steps;
    }
    hsd::println("u16 total saturated after {} additions at {}"_fmt, steps, total);

    // Floating point has no wrap-around, it saturates to infinity on its own.
    hsd::f32 huge = hsd::limits<hsd::f32>::max;
    hsd::f32 doubled = huge * 2;
    hsd::println("f32 max doubled is infinity: {}"_fmt,
        doubled == hsd::limits<hsd::f32>::infinity ? "true" : "false");

    return 0;
}
