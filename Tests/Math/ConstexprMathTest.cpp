#include <cassert>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <limits>
#include <type_traits>

#include <Base/Math.hpp>

// Runtime checks for the constexpr_math reference implementation. The public
// math:: functions call std at run time, so these calls are the only runtime
// coverage of the constexpr path. Each result is compared with the standard
// library. Special inputs (NaN, infinities, signed zero) are compared as well.

namespace cm = hsd::math::constexpr_math;
namespace detail = hsd::math::constexpr_math::ctmath_detail;
using hsd::math::constants::pi;

template <typename T>
constexpr long double tolerance()
{
    if constexpr (std::is_same_v<T, float>)
    {
        return 1e-5L;
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        return 1e-10L;
    }
    else
    {
        return 1e-12L;
    }
}

// `scale` loosens the bound for inputs where the function is ill-conditioned, such as
// tan close to a pole, where rounding of the input is magnified in the result.
template <typename T>
void expect_value(const char* name, T actual, T expected, long double scale = 1.0L)
{
    const bool ok = std::isnan(expected) ? std::isnan(actual)
        : std::isinf(expected) ? actual == expected
        : std::fabs(static_cast<long double>(actual) - static_cast<long double>(expected)) <=
            scale * tolerance<T>() * std::fmax(1.0L, std::fabs(static_cast<long double>(expected)));

    if (!ok)
    {
        std::fprintf(
            stderr, "%s: got %.17Lg, expected %.17Lg\n", name,
            static_cast<long double>(actual), static_cast<long double>(expected)
        );
    }
    assert(ok);
}

// Regression checks for the bugs fixed alongside these tests.
template <typename T>
void check_bug_regressions()
{
    // sqrt must converge for large and small inputs, and return zero for zero instead of NaN.
    expect_value<T>("sqrt(0)", cm::sqrt(static_cast<T>(0)), static_cast<T>(0));
    expect_value<T>("sqrt(1e6)", cm::sqrt(static_cast<T>(1e6)), static_cast<T>(1000));
    expect_value<T>("sqrt(1e-6)", cm::sqrt(static_cast<T>(1e-6)), static_cast<T>(1e-3));
    expect_value<T>("sqrt(2)", cm::sqrt(static_cast<T>(2)), std::sqrt(static_cast<T>(2)));
    expect_value<T>("sqrt(inf)", cm::sqrt(std::numeric_limits<T>::infinity()),
        std::numeric_limits<T>::infinity());
    expect_value<T>("sqrt(-1)", cm::sqrt(static_cast<T>(-1)), std::numeric_limits<T>::quiet_NaN());

    // acos(-x) is pi - acos(x).
    expect_value<T>("acos(0)", cm::acos(static_cast<T>(0)), static_cast<T>(pi / 2));
    expect_value<T>("acos(-0.5)", cm::acos(static_cast<T>(-0.5)), static_cast<T>(2 * pi / 3));
    expect_value<T>("acos(-1)", cm::acos(static_cast<T>(-1)), static_cast<T>(pi));

    // round rounds halves away from zero on both sides.
    expect_value<T>("round(-0.5)", cm::round(static_cast<T>(-0.5)), static_cast<T>(-1));
    expect_value<T>("round(-3.5)", cm::round(static_cast<T>(-3.5)), static_cast<T>(-4));
    expect_value<T>("round(-1000.5)", cm::round(static_cast<T>(-1000.5)), static_cast<T>(-1001));
    expect_value<T>("round(2.5)", cm::round(static_cast<T>(2.5)), static_cast<T>(3));
    expect_value<T>("round(-2.4)", cm::round(static_cast<T>(-2.4)), static_cast<T>(-2));

    // tanh saturates to +-1 for large magnitudes.
    expect_value<T>("tanh(1e6)", cm::tanh(static_cast<T>(1e6)), static_cast<T>(1));
    expect_value<T>("tanh(-1e6)", cm::tanh(static_cast<T>(-1e6)), static_cast<T>(-1));
    expect_value<T>("tanh(-1000.5)", cm::tanh(static_cast<T>(-1000.5)), static_cast<T>(-1));

    // asinh is odd, so large inputs of either sign must not cancel or overflow.
    expect_value<T>("asinh(-1e6)", cm::asinh(static_cast<T>(-1e6)), std::asinh(static_cast<T>(-1e6)));
    expect_value<T>("asinh(-20.25)", cm::asinh(static_cast<T>(-20.25)), std::asinh(static_cast<T>(-20.25)));
    expect_value<T>("asinh(1e20)", cm::asinh(static_cast<T>(1e20)), std::asinh(static_cast<T>(1e20)));
}

template <typename T>
void check_trig()
{
    for (const T value : {
        static_cast<T>(-20.25), static_cast<T>(-7.5), static_cast<T>(-3.5), static_cast<T>(-2.0),
        static_cast<T>(-1.25), static_cast<T>(-1.0), static_cast<T>(-0.75), static_cast<T>(-0.5),
        static_cast<T>(-0.001), static_cast<T>(0.001), static_cast<T>(0.25), static_cast<T>(0.5),
        static_cast<T>(0.75), static_cast<T>(1.0), static_cast<T>(1.2), static_cast<T>(1.4),
        static_cast<T>(1.45), static_cast<T>(1.5), static_cast<T>(1.62),
        static_cast<T>(1.7), static_cast<T>(2.0), static_cast<T>(2.5), static_cast<T>(3.0),
        static_cast<T>(3.2), static_cast<T>(3.5), static_cast<T>(4.0), static_cast<T>(6.0),
        static_cast<T>(7.5), static_cast<T>(20.25)})
    {
        expect_value<T>("sin", cm::sin(value), std::sin(value));
        expect_value<T>("cos", cm::cos(value), std::cos(value));

        expect_value<T>("tan", cm::tan(value), std::tan(value));
    }

    // Large arguments are reduced modulo pi, and the reduction must keep the precision of T.
    for (const T value : {static_cast<T>(-1e6), static_cast<T>(-1000.5), static_cast<T>(1000.5),
        static_cast<T>(123456.5), static_cast<T>(1e6)})
    {
        expect_value<T>("sin(large)", cm::sin(value), std::sin(value));
        expect_value<T>("cos(large)", cm::cos(value), std::cos(value));
        expect_value<T>("tan(large)", cm::tan(value), std::tan(value), 10.0L);
    }

    // Values between 1.55 and 1.6 use the Laurent series around the pole at pi/2. They stay
    // away from pi/2 itself, where the rounding of pi/2 in T dominates the result.
    for (const T value : {static_cast<T>(1.55), static_cast<T>(1.56), static_cast<T>(1.58), static_cast<T>(1.59)})
    {
        expect_value<T>("tan(pole)", cm::tan(value), std::tan(value), 10.0L);
    }

    // atan uses a different series or continued fraction for each magnitude range.
    for (const T value : {
        static_cast<T>(-9000.0), static_cast<T>(-1000.0), static_cast<T>(-50.0), static_cast<T>(-20.25),
        static_cast<T>(-9.0), static_cast<T>(-5.0), static_cast<T>(-3.5), static_cast<T>(-2.6),
        static_cast<T>(-2.2), static_cast<T>(-1.8), static_cast<T>(-1.2), static_cast<T>(-1.0),
        static_cast<T>(-0.75), static_cast<T>(-0.25), static_cast<T>(-0.001), static_cast<T>(0.0),
        static_cast<T>(0.001), static_cast<T>(0.25), static_cast<T>(0.75), static_cast<T>(1.0),
        static_cast<T>(1.2), static_cast<T>(1.8), static_cast<T>(2.2), static_cast<T>(2.6),
        static_cast<T>(3.5), static_cast<T>(4.5), static_cast<T>(6.0), static_cast<T>(9.0),
        static_cast<T>(15.0), static_cast<T>(50.0), static_cast<T>(500.0), static_cast<T>(5000.0)})
    {
        expect_value<T>("atan", cm::atan(value), std::atan(value));
        expect_value<T>("atan2", cm::atan2(value, static_cast<T>(2.5)), std::atan2(value, static_cast<T>(2.5)));
        expect_value<T>("atan2", cm::atan2(static_cast<T>(-3.0), value), std::atan2(static_cast<T>(-3.0), value));
    }

    // atan2 distinguishes signed zeros and the axis directions.
    const T zero = static_cast<T>(0);
    const T negative_zero = static_cast<T>(-0.0);
    for (const T y : {zero, negative_zero, static_cast<T>(2.0), static_cast<T>(-2.0)})
    {
        for (const T x : {zero, negative_zero, static_cast<T>(2.0), static_cast<T>(-2.0)})
        {
            expect_value<T>("atan2 axis", cm::atan2(y, x), std::atan2(y, x));
        }
    }

    for (const T value : {
        static_cast<T>(-1.0), static_cast<T>(-0.9), static_cast<T>(-0.5), static_cast<T>(-0.25),
        static_cast<T>(-0.001), static_cast<T>(0.0), static_cast<T>(0.001), static_cast<T>(0.25),
        static_cast<T>(0.5), static_cast<T>(0.323), static_cast<T>(0.9), static_cast<T>(1.0)})
    {
        expect_value<T>("asin", cm::asin(value), std::asin(value));
        expect_value<T>("acos", cm::acos(value), std::acos(value));
    }
}

template <typename T>
void check_hyperbolic()
{
    for (const T value : {
        static_cast<T>(-20.25), static_cast<T>(-3.5), static_cast<T>(-1.25), static_cast<T>(-0.5),
        static_cast<T>(-0.001), static_cast<T>(0.001), static_cast<T>(0.25), static_cast<T>(0.5),
        static_cast<T>(1.25), static_cast<T>(2.25), static_cast<T>(3.5), static_cast<T>(7.5),
        static_cast<T>(20.25)})
    {
        expect_value<T>("sinh", cm::sinh(value), std::sinh(value));
        expect_value<T>("cosh", cm::cosh(value), std::cosh(value));
        expect_value<T>("tanh", cm::tanh(value), std::tanh(value));
        expect_value<T>("asinh", cm::asinh(value), std::asinh(value));
    }

    for (const T value : {static_cast<T>(1.0), static_cast<T>(1.001), static_cast<T>(2.25), static_cast<T>(10.5)})
    {
        expect_value<T>("acosh", cm::acosh(value), std::acosh(value));
    }

    for (const T value : {static_cast<T>(-0.9), static_cast<T>(-0.22), static_cast<T>(0.0),
        static_cast<T>(0.22), static_cast<T>(0.9)})
    {
        expect_value<T>("atanh", cm::atanh(value), std::atanh(value));
    }
}

template <typename T>
void check_exponential_and_power()
{
    for (const T value : {
        static_cast<T>(-20.25), static_cast<T>(-3.5), static_cast<T>(-1.0), static_cast<T>(-0.25),
        static_cast<T>(0.0), static_cast<T>(0.5), static_cast<T>(1.0), static_cast<T>(2.5),
        static_cast<T>(7.5), static_cast<T>(20.25)})
    {
        expect_value<T>("exp", cm::exp(value), std::exp(value));
    }

    for (const T value : {
        static_cast<T>(0.001), static_cast<T>(0.125), static_cast<T>(0.5), static_cast<T>(1.0),
        static_cast<T>(2.0), static_cast<T>(5.0), static_cast<T>(20.0), static_cast<T>(1e6)})
    {
        expect_value<T>("log", cm::log(value), std::log(value));
        expect_value<T>("log2", cm::log2(value), std::log2(value));
        expect_value<T>("log10", cm::log10(value), std::log10(value));
        expect_value<T>("sqrt", cm::sqrt(value), std::sqrt(value));
        expect_value<T>("pow(x,1.5)", cm::pow(value, static_cast<T>(1.5)), std::pow(value, static_cast<T>(1.5)));
        expect_value<T>("pow(x,-2)", cm::pow(value, static_cast<T>(-2)), std::pow(value, static_cast<T>(-2)));
        expect_value<T>("pow(x,3)", cm::pow(value, 3), std::pow(value, static_cast<T>(3)));
        expect_value<T>("pow(x,0)", cm::pow(value, 0), static_cast<T>(1));
    }

    for (const hsd::usize count : {hsd::usize {0}, hsd::usize {1}, hsd::usize {2}, hsd::usize {5}, hsd::usize {10}, hsd::usize {20}})
    {
        T expected = 1;
        for (hsd::usize index = 2; index <= count; ++index)
        {
            expected *= static_cast<T>(index);
        }
        expect_value<T>("factorial", cm::factorial<T>(count), expected);
    }

    expect_value<T>("pow(2,3)", hsd::math::pow(static_cast<T>(2), 3), static_cast<T>(8));
    expect_value<T>("pow(2,0.5)", hsd::math::pow(static_cast<T>(2), static_cast<T>(0.5)), std::sqrt(static_cast<T>(2)));

    for (const T value : {static_cast<T>(-2.5), static_cast<T>(-1.0), static_cast<T>(-0.5), static_cast<T>(0.5),
        static_cast<T>(1.0), static_cast<T>(2.0), static_cast<T>(7.5), static_cast<T>(20.25)})
    {
        expect_value<T>("pow(-x,2)", cm::pow(value, 2), std::pow(value, static_cast<T>(2)));
        expect_value<T>("pow(x,4)", cm::pow(value, 4), std::pow(value, static_cast<T>(4)));
    }

    for (const T value : {static_cast<T>(0.5), static_cast<T>(1.0), static_cast<T>(2.5), static_cast<T>(4.75),
        static_cast<T>(7.5), static_cast<T>(20.25)})
    {
        expect_value<T>("lgamma", cm::lgamma(value), std::lgamma(value));
        expect_value<T>("tgamma", cm::tgamma(value), std::tgamma(value));
    }
}

template <typename T>
void check_rounding()
{
    for (const T value : {
        static_cast<T>(-1000.5), static_cast<T>(-20.25), static_cast<T>(-3.5), static_cast<T>(-1.0),
        static_cast<T>(-0.75), static_cast<T>(-0.5), static_cast<T>(-0.25), static_cast<T>(0.0),
        static_cast<T>(0.25), static_cast<T>(0.5), static_cast<T>(0.75), static_cast<T>(1.0),
        static_cast<T>(2.5), static_cast<T>(3.25), static_cast<T>(20.25), static_cast<T>(100.5)})
    {
        expect_value<T>("floor", cm::floor(value), std::floor(value));
        expect_value<T>("ceil", cm::ceil(value), std::ceil(value));
        expect_value<T>("trunc", cm::trunc(value), std::trunc(value));
        expect_value<T>("round", cm::round(value), std::round(value));
        expect_value<T>("mod", cm::mod(value, static_cast<T>(1.5)), std::fmod(value, static_cast<T>(1.5)));
    }
}

template <typename T>
void check_special_values()
{
    const T inf = std::numeric_limits<T>::infinity();
    const T nan = std::numeric_limits<T>::quiet_NaN();

    for (const T value : {inf, -inf, nan, static_cast<T>(-0.0), static_cast<T>(0.0)})
    {
        expect_value<T>("exp", cm::exp(value), std::exp(value));
        expect_value<T>("sin", cm::sin(value), std::sin(value));
        expect_value<T>("cos", cm::cos(value), std::cos(value));
        expect_value<T>("atan", cm::atan(value), std::atan(value));
        expect_value<T>("sinh", cm::sinh(value), std::sinh(value));
        expect_value<T>("cosh", cm::cosh(value), std::cosh(value));
        expect_value<T>("tanh", cm::tanh(value), std::tanh(value));
        expect_value<T>("asinh", cm::asinh(value), std::asinh(value));
        expect_value<T>("floor", cm::floor(value), std::floor(value));
        expect_value<T>("ceil", cm::ceil(value), std::ceil(value));
        expect_value<T>("round", cm::round(value), std::round(value));
        expect_value<T>("trunc", cm::trunc(value), std::trunc(value));
    }
}

// Domain edges: out-of-range arguments, NaN, infinities, signed zeros and the
// integer exponent limits. These reach the special-value branches of each function.
template <typename T>
void check_domain_edges()
{
    const T inf = std::numeric_limits<T>::infinity();
    const T nan = std::numeric_limits<T>::quiet_NaN();
    const T zero = static_cast<T>(0);
    const T negative_zero = static_cast<T>(-0.0);

    for (const T value : {nan, inf, -inf, zero, negative_zero})
    {
        expect_value<T>("tan", cm::tan(value), std::tan(value));
        expect_value<T>("asin", cm::asin(value), std::asin(value));
        expect_value<T>("acos", cm::acos(value), std::acos(value));
        expect_value<T>("acosh", cm::acosh(value), std::acosh(value));
        expect_value<T>("atanh", cm::atanh(value), std::atanh(value));
        expect_value<T>("log", cm::log(value), std::log(value));
        expect_value<T>("log2", cm::log2(value), std::log2(value));
        expect_value<T>("log10", cm::log10(value), std::log10(value));
        expect_value<T>("lgamma", cm::lgamma(value), std::lgamma(value));
        expect_value<T>("tgamma", cm::tgamma(value), std::tgamma(value));
        expect_value<T>("sqrt", cm::sqrt(value), std::sqrt(value));
        expect_value<T>("pow(x,0.5)", cm::pow(value, static_cast<T>(0.5)), std::pow(value, static_cast<T>(0.5)));
    }

    for (const T value : {static_cast<T>(-2.0), static_cast<T>(-1.0), static_cast<T>(-0.5),
        static_cast<T>(0.5), static_cast<T>(1.5), static_cast<T>(2.0)})
    {
        expect_value<T>("asin(out of range)", cm::asin(value), std::asin(value));
        expect_value<T>("acos(out of range)", cm::acos(value), std::acos(value));
        expect_value<T>("acosh(out of range)", cm::acosh(value), std::acosh(value));
        expect_value<T>("atanh(out of range)", cm::atanh(value), std::atanh(value));
    }

    expect_value<T>("log(-1)", cm::log(static_cast<T>(-1)), std::log(static_cast<T>(-1)));
    expect_value<T>("log(0)", cm::log(zero), std::log(zero));
    expect_value<T>("log2(0)", cm::log2(zero), std::log2(zero));
    expect_value<T>("log10(0)", cm::log10(zero), std::log10(zero));
    expect_value<T>("lgamma(0)", cm::lgamma(zero), std::lgamma(zero));
    expect_value<T>("lgamma(-1)", cm::lgamma(static_cast<T>(-1)), std::lgamma(static_cast<T>(-1)));
    expect_value<T>("tgamma(-1)", cm::tgamma(static_cast<T>(-1)), std::tgamma(static_cast<T>(-1)));
    expect_value<T>("tgamma(-0.5)", cm::tgamma(static_cast<T>(-0.5)), std::tgamma(static_cast<T>(-0.5)));
    expect_value<T>("tgamma(-1.5)", cm::tgamma(static_cast<T>(-1.5)), std::tgamma(static_cast<T>(-1.5)));
    expect_value<T>("tgamma(0.5)", cm::tgamma(static_cast<T>(0.5)), std::tgamma(static_cast<T>(0.5)));
    expect_value<T>("tgamma(1)", cm::tgamma(static_cast<T>(1)), std::tgamma(static_cast<T>(1)));
    expect_value<T>("tgamma(2)", cm::tgamma(static_cast<T>(2)), std::tgamma(static_cast<T>(2)));
    expect_value<T>("tgamma(12)", cm::tgamma(static_cast<T>(12)), std::tgamma(static_cast<T>(12)));
    expect_value<T>("pow(-2,0.5)", cm::pow(static_cast<T>(-2), static_cast<T>(0.5)), std::pow(static_cast<T>(-2), static_cast<T>(0.5)));

    // Integer exponents 1 and 0 return the base or one; the limits saturate.
    expect_value<T>("pow(x,1)", cm::pow(static_cast<T>(2.5), 1), static_cast<T>(2.5));
    expect_value<T>("pow(x,-1)", cm::pow(static_cast<T>(4), -1), static_cast<T>(0.25));
    expect_value<T>("pow(x,-3)", cm::pow(static_cast<T>(2), -3), static_cast<T>(0.125));
    expect_value<T>("pow(x,INT_MIN)", cm::pow(static_cast<T>(2), std::numeric_limits<int>::min()), zero);
    expect_value<T>("pow(x,INT_MAX)", cm::pow(static_cast<T>(2), std::numeric_limits<int>::max()), inf);

    // Trigonometric functions return exact values at the axis points.
    const T half_pi = static_cast<T>(pi / 2);
    expect_value<T>("sin(pi/2)", cm::sin(half_pi), std::sin(half_pi));
    expect_value<T>("sin(-pi/2)", cm::sin(-half_pi), std::sin(-half_pi));
    expect_value<T>("sin(pi)", cm::sin(static_cast<T>(pi)), std::sin(static_cast<T>(pi)));
    expect_value<T>("sin(-pi)", cm::sin(static_cast<T>(-pi)), std::sin(static_cast<T>(-pi)));
    expect_value<T>("cos(pi/2)", cm::cos(half_pi), std::cos(half_pi));
    expect_value<T>("cos(-pi/2)", cm::cos(-half_pi), std::cos(-half_pi));
    expect_value<T>("cos(pi)", cm::cos(static_cast<T>(pi)), std::cos(static_cast<T>(pi)));
    expect_value<T>("cos(-pi)", cm::cos(static_cast<T>(-pi)), std::cos(static_cast<T>(-pi)));
    expect_value<T>("tan(0)", cm::tan(zero), zero);
    expect_value<T>("tan(-0)", cm::tan(negative_zero), negative_zero);

    // Exponentials saturate for large arguments.
    expect_value<T>("exp(1000)", cm::exp(static_cast<T>(1000)), std::exp(static_cast<T>(1000)));
    expect_value<T>("exp(-1000)", cm::exp(static_cast<T>(-1000)), std::exp(static_cast<T>(-1000)));

    // Rounding functions pass large integral values and NaN/infinity through unchanged.
    for (const T value : {static_cast<T>(1e30), static_cast<T>(-1e30)})
    {
        expect_value<T>("floor(large)", cm::floor(value), std::floor(value));
        expect_value<T>("ceil(large)", cm::ceil(value), std::ceil(value));
        expect_value<T>("round(large)", cm::round(value), std::round(value));
        expect_value<T>("trunc(large)", cm::trunc(value), std::trunc(value));
    }
    expect_value<T>("mod(inf,1)", cm::mod(inf, static_cast<T>(1)), std::fmod(inf, static_cast<T>(1)));
    expect_value<T>("mod(nan,1)", cm::mod(nan, static_cast<T>(1)), std::fmod(nan, static_cast<T>(1)));

    // NaN propagates through the binary functions.
    expect_value<T>("atan2(nan,1)", cm::atan2(nan, static_cast<T>(1)), std::atan2(nan, static_cast<T>(1)));
    expect_value<T>("atan2(1,nan)", cm::atan2(static_cast<T>(1), nan), std::atan2(static_cast<T>(1), nan));

    // midpoint avoids overflow for large operands and underflow for subnormal ones.
    const T max = std::numeric_limits<T>::max();
    const T min = std::numeric_limits<T>::min();
    expect_value<T>("midpoint(max,max)", hsd::math::midpoint(max, max), max);
    expect_value<T>("midpoint(subnormal,1)", hsd::math::midpoint(min / 4, static_cast<T>(1)),
        static_cast<T>(min / 4 / 2 + static_cast<T>(1) / 2));
    expect_value<T>("midpoint(1,subnormal)", hsd::math::midpoint(static_cast<T>(1), min / 4),
        static_cast<T>(static_cast<T>(1) / 2 + min / 4 / 2));
    expect_value<T>("midpoint(1,3)", hsd::math::midpoint(static_cast<T>(1), static_cast<T>(3)), static_cast<T>(2));
}

// Branch edges that the checks above do not reach: argument order in min and max, the
// sign helper at zero, the integer-power helper with a zero exponent, the tangent pole,
// midpoint with one operand near the largest magnitude, and gamma functions at negative
// and large arguments.
template <typename T>
void check_branch_edges()
{
    const T zero = static_cast<T>(0);

    expect_value<T>("min(21,5)", hsd::math::min(static_cast<T>(21), static_cast<T>(5)), static_cast<T>(5));
    expect_value<T>("max(21,5)", hsd::math::max(static_cast<T>(21), static_cast<T>(5)), static_cast<T>(21));
    expect_value<T>("max(5,21)", hsd::math::max(static_cast<T>(5), static_cast<T>(21)), static_cast<T>(21));

    expect_value<T>("sgn(0)", detail::sgn(zero), zero);
    expect_value<T>("sgn(-2)", detail::sgn(static_cast<T>(-2)), static_cast<T>(-1));
    expect_value<T>("sgn(2)", detail::sgn(static_cast<T>(2)), static_cast<T>(1));

    expect_value<T>("pow_int_compute_rec(2,3,0)",
        detail::pow_int_compute_rec(static_cast<T>(2), static_cast<T>(3), 0), static_cast<T>(3));

    // A mantissa between 1 and 2 has no entry in the logarithm table.
    expect_value<T>("log(1.25)", cm::log(static_cast<T>(1.25)), std::log(static_cast<T>(1.25)));
    expect_value<T>("log(1.5)", cm::log(static_cast<T>(1.5)), std::log(static_cast<T>(1.5)));

    // tan is infinite at the pole. The sign follows the side of pi/2 that the argument is on,
    // which the long double constant cannot resolve, so only float and double check the sign.
    const T half_pi = static_cast<T>(hsd::math::constants::pi / 2);
    const T pole = cm::tan(half_pi);
    assert(std::isinf(pole));
    if constexpr (!std::is_same_v<T, long double>)
    {
        assert(std::signbit(pole) == std::signbit(std::tan(half_pi)));
    }

    const T max = std::numeric_limits<T>::max();
    const T min = std::numeric_limits<T>::min();
    expect_value<T>("midpoint(max,subnormal)", hsd::math::midpoint(max, min / 4), max / 2 + min / 4);
    expect_value<T>("midpoint(subnormal,max)", hsd::math::midpoint(min / 4, max), min / 4 + max / 2);

    expect_value<T>("pow(0,0.0)", cm::pow(zero, zero), static_cast<T>(1));
    expect_value<T>("pow(-2,3.0)", cm::pow(static_cast<T>(-2), static_cast<T>(3)), static_cast<T>(-8));
    expect_value<T>("pow(-2,2.0)", cm::pow(static_cast<T>(-2), static_cast<T>(2)), static_cast<T>(4));

    for (const T value : {static_cast<T>(-7.25), static_cast<T>(-2.5), static_cast<T>(-0.5)})
    {
        expect_value<T>("lgamma(negative)", cm::lgamma(value), std::lgamma(value));
    }
    expect_value<T>("lgamma(-3)", cm::lgamma(static_cast<T>(-3)), std::lgamma(static_cast<T>(-3)));

    // Integers above the factorial limit use exp(lgamma).
    expect_value<T>("tgamma(250)", cm::tgamma(static_cast<T>(250)), std::tgamma(static_cast<T>(250)));
}

template <typename T>
void check_type()
{
    check_bug_regressions<T>();
    check_trig<T>();
    check_hyperbolic<T>();
    check_exponential_and_power<T>();
    check_rounding<T>();
    check_special_values<T>();
    check_domain_edges<T>();
    check_branch_edges<T>();
}

int main()
{
    check_type<float>();
    check_type<double>();
    check_type<long double>();
}
