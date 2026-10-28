#include "TextCheck.hpp"
#include <Base/Limits.hpp>
#include <Serialize/ToString.hpp>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>

// Copies a conversion buffer into a null-terminated array for the C parsers.
template <typename Buf>
static void copy_text(const Buf& buf, char (&text)[max_text_length])
{
    for (hsd::usize _idx = 0; _idx < buf.size(); ++_idx)
    {
        text[_idx] = buf.data()[_idx];
    }

    text[buf.size()] = '\0';
}

// The C library reads hexadecimal floats exactly, so a correct conversion parses
// back to the value it came from. This check covers every type, including x87.
template <typename T>
static void expect_round_trip(T value)
{
    char _text[max_text_length] = {};
    copy_text(hsd::to_pseudo_string_hex(value), _text);

    T _parsed{};

    if constexpr (hsd::IsSame<T, float>)
    {
        _parsed = std::strtof(_text, nullptr);
    }
    else if constexpr (hsd::IsSame<T, double>)
    {
        _parsed = std::strtod(_text, nullptr);
    }
    else
    {
        _parsed = std::strtold(_text, nullptr);
    }

    assert(_parsed == value);
}

// C's `%a` writes a `+` before a non-negative exponent, and this project omits it.
// The reference drops that sign before the comparison.
static void expect_matches_printf_double(double value)
{
    char _reference[128] = {};
    std::snprintf(_reference, sizeof _reference, "%a", value);

    char _without_plus[128] = {};
    hsd::usize _length = 0;

    for (hsd::usize _idx = 0; _reference[_idx] != '\0'; ++_idx)
    {
        if (_reference[_idx] != '+')
        {
            _without_plus[_length++] = _reference[_idx];
        }
    }

    assert(equals(hsd::to_pseudo_string_hex(value), _without_plus));
}

static void check_known_values()
{
    assert(equals(hsd::to_pseudo_string_hex(1.0), "0x1p0"));
    assert(equals(hsd::to_pseudo_string_hex(-1.0), "-0x1p0"));
    assert(equals(hsd::to_pseudo_string_hex(0.5), "0x1p-1"));
    assert(equals(hsd::to_pseudo_string_hex(3.0), "0x1.8p1"));
    assert(equals(hsd::to_pseudo_string_hex(0.1), "0x1.999999999999ap-4"));
    assert(equals(hsd::to_pseudo_string_hex(0.0), "0x0p0"));
    assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f64>::denorm_min),
        "0x0.0000000000001p-1022"));
    assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f64>::max),
        "0x1.fffffffffffffp1023"));

    assert(equals(hsd::to_pseudo_string_hex(1.0f), "0x1p0"));
    assert(equals(hsd::to_pseudo_string_hex(0.1f), "0x1.99999ap-4"));
    assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f32>::max), "0x1.fffffep127"));
    assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f32>::denorm_min),
        "0x0.000002p-126"));

    assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f64>::infinity), "inf"));
    assert(equals(hsd::to_pseudo_string_hex(-hsd::limits<hsd::f64>::infinity), "-inf"));

    // x87 long double keeps an explicit integer bit, so its leading digit is normalized.
    if constexpr (hsd::limits<hsd::f128>::mantissa > hsd::limits<hsd::f64>::mantissa)
    {
        assert(equals(hsd::to_pseudo_string_hex(1.0L), "0x1p0"));
        assert(equals(hsd::to_pseudo_string_hex(0.5L), "0x1p-1"));
        assert(equals(hsd::to_pseudo_string_hex(0.1L), "0x1.999999999999999ap-4"));
        assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f128>::max),
            "0x1.fffffffffffffffep16383"));
        assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f128>::denorm_min),
            "0x0.0000000000000002p-16382"));
    }
}

// Constant evaluation takes the same code path, so it must give the same text.
static_assert(equals(hsd::to_pseudo_string_hex(1.0), "0x1p0"));
static_assert(equals(hsd::to_pseudo_string_hex(3.0), "0x1.8p1"));
static_assert(equals(hsd::to_pseudo_string_hex(0.0), "0x0p0"));
static_assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f64>::denorm_min),
    "0x0.0000000000001p-1022"));
static_assert(equals(hsd::to_pseudo_string_hex(hsd::limits<hsd::f64>::max),
    "0x1.fffffffffffffp1023"));

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

        expect_round_trip(_value);
        expect_matches_printf_double(_value);
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

        expect_round_trip(_value);
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

        expect_round_trip(_value);
    }
}

int main()
{
    check_known_values();

    std::mt19937_64 rng{0x5EED'0002u};
    check_random_doubles(rng, 20'000);
    check_random_floats(rng, 10'000);

    if constexpr (hsd::limits<hsd::f128>::mantissa > hsd::limits<hsd::f64>::mantissa)
    {
        check_random_long_doubles(rng, 2'000);

        // The longest text is a negative subnormal with a six-character exponent.
        expect_round_trip(-hsd::limits<hsd::f128>::denorm_min);
        expect_round_trip(-hsd::limits<hsd::f128>::max);
    }

    return 0;
}
