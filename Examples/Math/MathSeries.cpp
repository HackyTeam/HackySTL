// Title: Series expansions
// Level: Intermediate
// Shows: Taylor and Leibniz series compared against hsd::math, plus constexpr factorial.

#include <Base/Math.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// e^x = 1 + x + x^2/2! + x^3/3! + ...
static hsd::f64 exp_series(hsd::f64 x, hsd::u32 terms)
{
    hsd::f64 term = 1.0;
    hsd::f64 sum = 1.0;

    for (hsd::u32 index = 1; index < terms; ++index)
    {
        term = term * x / index;
        sum += term;
    }

    return sum;
}

// sin(x) = x - x^3/3! + x^5/5! - ...
static hsd::f64 sin_series(hsd::f64 x, hsd::u32 terms)
{
    hsd::f64 term = x;
    hsd::f64 sum = x;

    for (hsd::u32 index = 1; index < terms; ++index)
    {
        term = -term * x * x / ((2 * index) * (2 * index + 1));
        sum += term;
    }

    return sum;
}

// pi/4 = 1 - 1/3 + 1/5 - 1/7 + ...
static hsd::f64 leibniz_pi(hsd::u32 terms)
{
    hsd::f64 sum = 0.0;
    hsd::f64 sign = 1.0;

    for (hsd::u32 index = 0; index < terms; ++index)
    {
        sum += sign / (2 * index + 1);
        sign = -sign;
    }

    return 4.0 * sum;
}

int main()
{
    hsd::println("terms  series        library"_fmt);
    const hsd::u32 counts[]{2, 4, 8, 12};

    for (hsd::u32 terms : counts)
    {
        hsd::println("{}  exp(1.5)={prec=6}  {prec=6}"_fmt,
            terms, exp_series(1.5, terms), hsd::math::exp(1.5));
    }

    hsd::println("sin(1.2): series={prec=8} library={prec=8}"_fmt,
        sin_series(1.2, 10), hsd::math::sin(1.2));

    // The series error shrinks with more terms, which is what we check.
    hsd::f64 reference = hsd::math::exp(1.5);
    hsd::f64 error_few = hsd::math::abs(exp_series(1.5, 4) - reference);
    hsd::f64 error_many = hsd::math::abs(exp_series(1.5, 12) - reference);
    hsd::println("more terms are closer: {}"_fmt, error_many < error_few ? "true" : "false");

    // The Leibniz series for pi converges slowly: one more digit per ten times the terms.
    const hsd::u32 big_counts[]{10, 1000, 100000};
    for (hsd::u32 terms : big_counts)
    {
        hsd::println("leibniz({}) = {prec=4}"_fmt, terms, leibniz_pi(terms));
    }

    // factorial is constexpr, so a table of it can be built at compile time.
    constexpr hsd::f64 fact_5 = hsd::math::constexpr_math::factorial<hsd::f64>(5);
    constexpr hsd::f64 fact_10 = hsd::math::constexpr_math::factorial<hsd::f64>(10);
    hsd::println("5!={prec=0} 10!={prec=0}"_fmt, fact_5, fact_10);

    // Stirling's approximation of n! is a series-like closed form to compare with.
    hsd::f64 pi = static_cast<hsd::f64>(hsd::math::constants::pi);
    hsd::f64 stirling = hsd::math::sqrt(2 * pi * 10) *
        hsd::math::pow(10 / hsd::math::exp(1.0), 10);
    hsd::println("stirling(10) is within 1 percent of 10!: {}"_fmt,
        hsd::math::abs(stirling - fact_10) < 0.01 * fact_10 ? "true" : "false");

    return 0;
}
