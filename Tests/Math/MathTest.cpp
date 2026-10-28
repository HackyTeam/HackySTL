#include <cassert>
#include <cmath>
#include <initializer_list>

#include <Base/Math.hpp>

static void expect_close(double actual, double expected)
{
    const double error = std::abs(actual - expected);
    const double scale = std::max(1.0, std::abs(expected));
    assert(error <= 1e-12 * scale);
}

int main()
{
    for (const double value : {-12.5, -1.25, -0.0, 0.0, 0.5, 2.25, 12.5})
    {
        expect_close(hsd::math::abs(value), std::abs(value));
        expect_close(hsd::math::ceil(value), std::ceil(value));
        expect_close(hsd::math::floor(value), std::floor(value));
        expect_close(hsd::math::round(value), std::round(value));
        expect_close(hsd::math::sin(value), std::sin(value));
        expect_close(hsd::math::cos(value), std::cos(value));
        expect_close(hsd::math::tan(value), std::tan(value));
    }

    for (const double value : {0.125, 0.5, 1.0, 2.0, 5.0, 20.0})
    {
        expect_close(hsd::math::sqrt(value), std::sqrt(value));
        expect_close(hsd::math::exp(value / 3.0), std::exp(value / 3.0));
        expect_close(hsd::math::log(value), std::log(value));
        expect_close(hsd::math::log2(value), std::log2(value));
        expect_close(hsd::math::log10(value), std::log10(value));
        expect_close(hsd::math::pow(value, 1.5), std::pow(value, 1.5));
    }

    expect_close(hsd::math::asin(0.323), std::asin(0.323));
    expect_close(hsd::math::acos(0.323), std::acos(0.323));
    expect_close(hsd::math::atan(5.0), std::atan(5.0));
    expect_close(hsd::math::atan2(3.0, -2.0), std::atan2(3.0, -2.0));
    expect_close(hsd::math::sinh(1.25), std::sinh(1.25));
    expect_close(hsd::math::cosh(1.25), std::cosh(1.25));
    expect_close(hsd::math::tanh(1.25), std::tanh(1.25));
    expect_close(hsd::math::asinh(1.25), std::asinh(1.25));
    expect_close(hsd::math::acosh(2.25), std::acosh(2.25));
    expect_close(hsd::math::atanh(0.22), std::atanh(0.22));
    expect_close(hsd::math::lgamma(5.0), std::lgamma(5.0));
    expect_close(hsd::math::tgamma(5.33), std::tgamma(5.33));
    expect_close(hsd::math::mod(5.0, 1.3), std::fmod(5.0, 1.3));

    assert(hsd::math::min(5.0, 21.0) == 5.0);
    assert(hsd::math::max(5.0, 21.0) == 21.0);
}
