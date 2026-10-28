// Title: Math basics
// Level: Basic
// Shows: rounding, powers, roots, trigonometry and the pi constant from hsd::math.

#include <Base/Math.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    // Rounding family.
    hsd::f64 value = 2.75;
    hsd::println("floor={prec=1} ceil={prec=1} round={prec=1} trunc={prec=1}"_fmt,
        hsd::math::floor(value), hsd::math::ceil(value),
        hsd::math::round(value), hsd::math::trunc(value));
    hsd::println("abs(-3.5)={prec=1} min(4,9)={prec=1} max(4,9)={prec=1}"_fmt,
        hsd::math::abs(-3.5), hsd::math::min(4.0, 9.0), hsd::math::max(4.0, 9.0));

    // Powers, roots and logarithms.
    hsd::println("sqrt(2)={prec=4}"_fmt, hsd::math::sqrt(2.0));
    hsd::println("pow(2, 10)={prec=1} pow(9, 0.5)={prec=1}"_fmt,
        hsd::math::pow(2.0, 10), hsd::math::pow(9.0, 0.5));
    hsd::println("log2(1024)={prec=1} log10(1000)={prec=1}"_fmt,
        hsd::math::log2(1024.0), hsd::math::log10(1000.0));
    hsd::println("exp(1)={prec=4} mod(7.5, 2)={prec=1}"_fmt,
        hsd::math::exp(1.0), hsd::math::mod(7.5, 2.0));

    // Constants are stored as f128, so convert them before use.
    hsd::f64 pi = static_cast<hsd::f64>(hsd::math::constants::pi);
    hsd::println("pi is {prec=5}"_fmt, pi);

    // Trigonometry works in radians.
    hsd::println("sin(pi/2)={prec=3} cos(pi)={prec=3} tan(pi/4)={prec=3}"_fmt,
        hsd::math::sin(pi / 2), hsd::math::cos(pi), hsd::math::tan(pi / 4));
    hsd::println("atan2(1, 1)={prec=4}"_fmt, hsd::math::atan2(1.0, 1.0));

    return 0;
}
