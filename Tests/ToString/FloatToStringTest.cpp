#include "TextCheck.hpp"
#include <Base/Limits.hpp>
#include <Base/Utility.hpp>
#include <Serialize/ToString.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <random>

// The C library is the reference. Its %f and %e output is correctly rounded.
template <hsd::usize Precision, typename T>
static void expect_fixed(T value)
{
    char _expected[max_text_length] = {};

    if constexpr (hsd::IsSame<T, hsd::f128>)
    {
        std::snprintf(_expected, sizeof _expected, "%.*Lf", static_cast<int>(Precision), value);
    }
    else
    {
        const double _widened = static_cast<double>(value);
        std::snprintf(_expected, sizeof _expected, "%.*f", static_cast<int>(Precision), _widened);
    }

    assert(equals(hsd::to_pseudo_string<char, Precision>(value), _expected));
}

template <hsd::usize Precision, typename T>
static void expect_scientific(T value)
{
    char _expected[max_text_length] = {};

    if constexpr (hsd::IsSame<T, hsd::f128>)
    {
        std::snprintf(_expected, sizeof _expected, "%.*Le", static_cast<int>(Precision), value);
    }
    else
    {
        const double _widened = static_cast<double>(value);
        std::snprintf(_expected, sizeof _expected, "%.*e", static_cast<int>(Precision), _widened);
    }

    assert(equals(hsd::to_pseudo_string_exp<char, Precision>(value), _expected));
}

static void check_known_values()
{
    // Ties go to the even neighbour. Both inputs are exact binary fractions.
    assert(equals(hsd::to_pseudo_string(0.0078125), "0.007812"));
    assert(equals(hsd::to_pseudo_string(0.0234375), "0.023438"));
    assert(equals(hsd::to_pseudo_string<char, 0>(0.5), "0"));
    assert(equals(hsd::to_pseudo_string<char, 0>(2.5), "2"));
    assert(equals(hsd::to_pseudo_string<char, 0>(3.5), "4"));

    // Nearest rounding, including carries into the integral part.
    assert(equals(hsd::to_pseudo_string(2.675), "2.675000"));
    assert(equals(hsd::to_pseudo_string(0.9999995), "1.000000"));
    assert(equals(hsd::to_pseudo_string(9.9999996), "10.000000"));
    assert(equals(hsd::to_pseudo_string(-1.5), "-1.500000"));
    assert(equals(hsd::to_pseudo_string(0.0), "0.000000"));
    assert(equals(hsd::to_pseudo_string(1e22), "10000000000000000000000.000000"));

    assert(equals(hsd::to_pseudo_string_exp(12345.678), "1.234568e+04"));
    assert(equals(hsd::to_pseudo_string_exp(0.9999995), "9.999995e-01"));
    assert(equals(hsd::to_pseudo_string_exp(9.9999996), "1.000000e+01"));
    assert(equals(hsd::to_pseudo_string_exp(-0.000123456789), "-1.234568e-04"));
    assert(equals(hsd::to_pseudo_string_exp(0.0), "0.000000e+00"));
}

static void check_special_values()
{
    assert(equals(hsd::to_pseudo_string(hsd::limits<hsd::f64>::infinity), "inf"));
    assert(equals(hsd::to_pseudo_string(-hsd::limits<hsd::f64>::infinity), "-inf"));
    assert(equals(hsd::to_pseudo_string_exp(hsd::limits<hsd::f64>::infinity), "inf"));
}

// Constant evaluation must produce the same text as the run-time calls above.
static_assert(equals(hsd::to_pseudo_string(2.675), "2.675000"));
static_assert(equals(hsd::to_pseudo_string(0.0078125), "0.007812"));
static_assert(equals(hsd::to_pseudo_string<char, 0>(2.5), "2"));
static_assert(equals(hsd::to_pseudo_string_exp(12345.678), "1.234568e+04"));
static_assert(equals(hsd::to_pseudo_string_exp(0.0), "0.000000e+00"));
static_assert(equals(
    hsd::to_pseudo_string_exp(hsd::limits<hsd::f64>::denorm_min), "4.940656e-324"));

// Random bit patterns reach every exponent, including subnormals and huge values.
static void check_random_doubles(std::mt19937_64& rng, int count)
{
    for (int _iteration = 0; _iteration < count; ++_iteration)
    {
        const hsd::u64 _bits = rng();
        const double _value = hsd::bit_cast<double>(_bits);

        // NaNs, infinities, and zeros are covered by the fixed checks above.
        if (((_bits >> 52) & 0x7FF) == 0x7FF || _value == 0)
        {
            continue;
        }

        expect_fixed<0>(_value);
        expect_fixed<6>(_value);
        expect_fixed<27>(_value);   // largest precision of the 128-bit fast path
        expect_fixed<28>(_value);   // first precision that uses the exact digit path
        expect_scientific<0>(_value);
        expect_scientific<6>(_value);
        expect_scientific<20>(_value);
    }
}

static void check_random_floats(std::mt19937_64& rng, int count)
{
    for (int _iteration = 0; _iteration < count; ++_iteration)
    {
        const hsd::u32 _bits = static_cast<hsd::u32>(rng());
        const float _value = hsd::bit_cast<float>(_bits);

        if (((_bits >> 23) & 0xFF) == 0xFF || _value == 0)
        {
            continue;
        }

        expect_fixed<6>(_value);
        expect_fixed<27>(_value);
        expect_scientific<6>(_value);
    }
}

// x87 long double: a 64-bit significand with the integer bit explicit.
static void check_random_long_doubles(std::mt19937_64& rng, int count)
{
    for (int _iteration = 0; _iteration < count; ++_iteration)
    {
        const hsd::u64 _significand = rng() | (hsd::u64{1} << 63);
        const int _exponent = static_cast<int>(rng() % 32000) - 16100;
        const long double _value =
            std::ldexp(static_cast<long double>(_significand), _exponent - 63);

        if (_value == 0)
        {
            continue;
        }

        expect_fixed<6>(_value);
        expect_fixed<27>(_value);
        expect_scientific<6>(_value);
        expect_scientific<20>(_value);
    }
}

int main()
{
    check_known_values();
    check_special_values();

    std::mt19937_64 rng{0x5EED'0001u};
    check_random_doubles(rng, 20'000);
    check_random_floats(rng, 10'000);

    expect_fixed<6>(hsd::limits<hsd::f64>::max);
    expect_scientific<6>(hsd::limits<hsd::f64>::max);
    expect_fixed<6>(hsd::limits<hsd::f64>::denorm_min);
    expect_scientific<20>(hsd::limits<hsd::f64>::denorm_min);
    expect_scientific<6>(hsd::limits<hsd::f32>::max);
    expect_fixed<6>(hsd::limits<hsd::f32>::denorm_min);

    // Long double is wider than double only on some targets; elsewhere it aliases double.
    if constexpr (hsd::limits<hsd::f128>::mantissa > hsd::limits<hsd::f64>::mantissa)
    {
        check_random_long_doubles(rng, 1'000);
        expect_fixed<6>(hsd::limits<hsd::f128>::max);
        expect_scientific<6>(hsd::limits<hsd::f128>::max);
        expect_fixed<6>(hsd::limits<hsd::f128>::denorm_min);
        expect_scientific<6>(hsd::limits<hsd::f128>::denorm_min);
    }

    return 0;
}
