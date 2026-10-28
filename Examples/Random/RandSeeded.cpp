// Title: Seeded random numbers
// Level: Basic
// Shows: mt19937_32 with a fixed seed, ranged and floating point draws, and reproducibility.

#include <Base/Random.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    // The same seed always gives the same sequence, which is what makes the
    // output of this program checkable. mt19937_32 matches std::mt19937.
    hsd::mt19937_32 gen{42};

    hsd::print("raw words:"_fmt);
    for (int count = 0; count < 3; ++count)
    {
        hsd::print(" {}"_fmt, gen.generate());
    }
    hsd::println(""_fmt);

    // generate(lo, hi) returns a result over [lo, hi), and fails if lo >= hi.
    hsd::print("dice:"_fmt);
    for (int count = 0; count < 10; ++count)
    {
        auto roll = gen.generate(1, 7);
        if (roll.is_ok())
        {
            hsd::print(" {}"_fmt, roll.unwrap());
        }
    }
    hsd::println(""_fmt);

    auto empty_range = gen.generate(5, 5);
    hsd::println("empty range ok: {}"_fmt, empty_range.is_ok() ? "true" : "false");
    hsd::println("reason: {}"_fmt, empty_range.unwrap_err().pretty_error());

    // With no bounds, a floating point type gives a value in [0, 1).
    hsd::f64 unit = gen.generate<hsd::f64>();
    hsd::println("unit in range: {}"_fmt, (unit >= 0.0 && unit < 1.0) ? "true" : "false");

    // Two engines built from the same seed stay in step.
    hsd::mt19937_32 first{2024};
    hsd::mt19937_32 second{2024};
    bool same = true;
    for (int count = 0; count < 100; ++count)
    {
        same = same && first.generate() == second.generate();
    }
    hsd::println("same seed, same sequence: {}"_fmt, same ? "true" : "false");

    return 0;
}
