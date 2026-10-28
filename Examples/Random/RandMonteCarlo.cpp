// Title: Monte Carlo estimates
// Level: Intermediate
// Shows: seeded floating point draws estimating pi, and a dice simulation with a histogram.

#include <Base/Random.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    hsd::mt19937_64 gen{7};

    // Throw darts at the unit square and count the ones inside the quarter circle.
    const hsd::u32 darts = 200000;
    hsd::u32 inside = 0;

    for (hsd::u32 throw_index = 0; throw_index < darts; ++throw_index)
    {
        hsd::f64 x = gen.generate<hsd::f64>();
        hsd::f64 y = gen.generate<hsd::f64>();

        if (x * x + y * y < 1.0)
        {
            ++inside;
        }
    }

    hsd::f64 estimate = 4.0 * inside / darts;
    hsd::println("darts={}"_fmt, darts);
    hsd::println("close to pi: {}"_fmt, (estimate > 3.12 && estimate < 3.16) ? "true" : "false");
    hsd::println("pi is about {prec=2}"_fmt, estimate);

    // Sum of two dice: the middle total is the most common one.
    hsd::mt19937_32 dice{99};
    hsd::u32 totals[13]{};
    const hsd::u32 rolls = 36000;

    for (hsd::u32 roll = 0; roll < rolls; ++roll)
    {
        hsd::i32 sum = dice.generate(1, 7).unwrap() + dice.generate(1, 7).unwrap();
        ++totals[sum];
    }

    hsd::u32 best = 2;
    hsd::u32 counted = 0;
    for (hsd::u32 sum = 2; sum <= 12; ++sum)
    {
        counted += totals[sum];
        if (totals[sum] > totals[best])
        {
            best = sum;
        }

        // One star stands for 500 rolls.
        hsd::print("{}:"_fmt, sum);
        for (hsd::u32 star = 0; star < totals[sum] / 500; ++star)
        {
            hsd::print("*"_fmt);
        }
        hsd::println(""_fmt);
    }

    hsd::println("all rolls counted: {}"_fmt, counted == rolls ? "true" : "false");
    hsd::println("most common total: {}"_fmt, best);

    return 0;
}
