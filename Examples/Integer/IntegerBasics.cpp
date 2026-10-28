// Title: Fixed-width integer basics
// Level: Basic
// Shows: integer<N> arithmetic that wraps modulo 2^N, division with remainder, and widening.

#include <Containers/Integer.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    // integer<8> has eight bits, so every result wraps modulo 256.
    hsd::integer<8> a = 250;
    hsd::integer<8> b = 10;
    hsd::println("8 bit: {} + {} = {}"_fmt, a, b, a + b);
    hsd::println("8 bit: {} - {} = {}"_fmt, b, a, b - a);
    hsd::println("8 bit: {} * {} = {}"_fmt, a, b, a * b);
    hsd::println("8 bit: {} / {} = {}"_fmt, a, b, a / b);

    // Counting below zero wraps to the largest value.
    hsd::integer<64> counter = 0;
    --counter;
    hsd::println("0 - 1 in 64 bits: {} ({hex})"_fmt, counter, counter);
    ++counter;
    hsd::println("and back up: {}"_fmt, counter);

    // Wider integers hold what a machine word cannot: 2^100.
    hsd::integer<128> power = 1;
    for (hsd::i32 step = 0; step < 100; ++step)
    {
        power *= 2;
    }
    hsd::println("2^100 = {}"_fmt, power);

    // divide_with_reminder gives both parts in a result, which fails on zero.
    hsd::integer<128> big = 1'000'000'007;
    hsd::integer<128> cube = big * big * big;
    hsd::println("cube: {}"_fmt, cube);

    auto division = cube.divide_with_reminder(hsd::integer<128>{12345});
    if (division.is_ok())
    {
        auto parts = division.unwrap();
        hsd::println("cube / 12345 = {} remainder {}"_fmt, parts.quotient, parts.reminder);
    }

    auto by_zero = cube.divide_with_reminder(hsd::integer<128>{0});
    hsd::println("division by zero ok: {}"_fmt, by_zero.is_ok() ? "true" : "false");

    // Integers compare with the usual operators and convert between widths.
    hsd::integer<32> small = 77;
    hsd::integer<128> widened = small;
    hsd::println("77 < cube: {}"_fmt, widened < cube ? "true" : "false");
    hsd::println("widened: {}"_fmt, widened);

    return 0;
}
