// Title: Numeric integration
// Level: Advanced
// Shows: trapezoid and Simpson rules, Newton's method, and the same maths evaluated at compile time.

#include <Base/Math.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// A function pointer type keeps the integrators generic without templates.
using real_function = hsd::f64 (*)(hsd::f64);

constexpr hsd::f64 trapezoid(real_function func, hsd::f64 from, hsd::f64 to, hsd::u32 steps)
{
    hsd::f64 width = (to - from) / steps;
    hsd::f64 sum = (func(from) + func(to)) / 2;

    for (hsd::u32 step = 1; step < steps; ++step)
    {
        sum += func(from + step * width);
    }

    return sum * width;
}

constexpr hsd::f64 simpson(real_function func, hsd::f64 from, hsd::f64 to, hsd::u32 steps)
{
    // Simpson's rule needs an even number of intervals.
    hsd::f64 width = (to - from) / steps;
    hsd::f64 sum = func(from) + func(to);

    for (hsd::u32 step = 1; step < steps; ++step)
    {
        sum += func(from + step * width) * (step % 2 == 1 ? 4 : 2);
    }

    return sum * width / 3;
}

constexpr hsd::f64 sine(hsd::f64 x)
{
    return hsd::math::sin(x);
}

constexpr hsd::f64 square(hsd::f64 x)
{
    return x * x;
}

constexpr hsd::f64 gaussian(hsd::f64 x)
{
    return hsd::math::exp(-x * x);
}

// Newton's method for sqrt(target): repeat x = (x + target / x) / 2.
constexpr hsd::f64 newton_sqrt(hsd::f64 target, hsd::u32 rounds)
{
    hsd::f64 guess = target;
    for (hsd::u32 round = 0; round < rounds; ++round)
    {
        guess = (guess + target / guess) / 2;
    }
    return guess;
}

constexpr hsd::f64 close_to(hsd::f64 value, hsd::f64 expected, hsd::f64 tolerance)
{
    return hsd::math::abs(value - expected) < tolerance;
}

// The integrals are exact enough to be checked while compiling.
constexpr hsd::f64 pi_value = 3.14159265358979323846;
static_assert(close_to(simpson(square, 0.0, 3.0, 100), 9.0, 1e-9));
static_assert(close_to(simpson(sine, 0.0, pi_value, 200), 2.0, 1e-6));
static_assert(close_to(newton_sqrt(2.0, 10), 1.41421356237, 1e-9));

int main()
{
    hsd::println("integral of sin over [0, pi] (exact 2):"_fmt);
    const hsd::u32 step_counts[]{4, 16, 64};
    for (hsd::u32 steps : step_counts)
    {
        hsd::println("  n={} trapezoid={prec=5} simpson={prec=5}"_fmt, steps,
            trapezoid(sine, 0.0, pi_value, steps), simpson(sine, 0.0, pi_value, steps));
    }

    hsd::println("integral of x^2 over [0, 3] (exact 9): {prec=6}"_fmt,
        simpson(square, 0.0, 3.0, 10));

    // The Gaussian integral over the whole line is sqrt(pi); the tails past +-6 are negligible.
    hsd::f64 area = simpson(gaussian, -6.0, 6.0, 600);
    hsd::println("gaussian area is {prec=6}, sqrt(pi) is {prec=6}"_fmt, area, hsd::math::sqrt(pi_value));

    // Simpson beats the trapezoid rule at the same number of steps.
    hsd::f64 trap_error = hsd::math::abs(trapezoid(sine, 0.0, pi_value, 16) - 2.0);
    hsd::f64 simp_error = hsd::math::abs(simpson(sine, 0.0, pi_value, 16) - 2.0);
    hsd::println("simpson is more accurate: {}"_fmt, simp_error < trap_error ? "true" : "false");

    hsd::println("newton sqrt(2) after 1, 2, 5 rounds:"_fmt);
    const hsd::u32 rounds[]{1, 2, 5};
    for (hsd::u32 count : rounds)
    {
        hsd::println("  {} -> {prec=8}"_fmt, count, newton_sqrt(2.0, count));
    }
    hsd::println("library sqrt(2)={prec=8}"_fmt, hsd::math::sqrt(2.0));

    return 0;
}
