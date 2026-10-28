#include "TextCheck.hpp"
#include <Base/Limits.hpp>
#include <Serialize/ToString.hpp>
#include <cassert>
#include <cstdio>
#include <random>

// Decimal text matches printf, widened to the largest integer of the same signedness.
template <typename T>
static void expect_decimal(T value)
{
    char _expected[64] = {};

    if constexpr (hsd::IsSigned<T>)
    {
        std::snprintf(_expected, sizeof _expected, "%lld", static_cast<long long>(value));
    }
    else
    {
        std::snprintf(_expected, sizeof _expected, "%llu", static_cast<unsigned long long>(value));
    }

    assert(equals(hsd::to_pseudo_string(value), _expected));
}

// Hexadecimal text is the value's bit pattern at its own width, so a negative value
// prints as its two's-complement digits.
template <typename T>
static void expect_hex(T value)
{
    char _expected[64] = {};
    const auto _unsigned = static_cast<hsd::add_unsigned_t<T>>(value);
    std::snprintf(
        _expected, sizeof _expected, "0x%llx", static_cast<unsigned long long>(_unsigned));

    assert(equals(hsd::to_pseudo_string_hex(value), _expected));
}

// The wide conversion writes the same characters as the narrow one.
template <typename T>
static void expect_wide_matches_narrow(T value)
{
    const auto _narrow = hsd::to_pseudo_string(value);
    const auto _wide = hsd::to_pseudo_string<hsd::wchar>(value);

    assert(_narrow.size() == _wide.size());

    for (hsd::usize _idx = 0; _idx < _narrow.size(); ++_idx)
    {
        assert(static_cast<char>(_wide.data()[_idx]) == _narrow.data()[_idx]);
    }
}

template <typename T>
static void check_integer_type(std::mt19937_64& rng)
{
    const T _values[] = {
        static_cast<T>(0), static_cast<T>(1), static_cast<T>(10),
        hsd::limits<T>::min, hsd::limits<T>::max,
    };

    for (const T _value : _values)
    {
        expect_decimal(_value);
        expect_hex(_value);
        expect_wide_matches_narrow(_value);
    }

    for (int _iteration = 0; _iteration < 2'000; ++_iteration)
    {
        const T _value = static_cast<T>(rng());
        expect_decimal(_value);
        expect_hex(_value);
    }
}

static void check_wide_integers()
{
    assert(equals(hsd::to_pseudo_string<hsd::wchar>(-42), L"-42"));
    assert(equals(hsd::to_pseudo_string<hsd::char8>(42), u8"42"));
    assert(equals(hsd::to_pseudo_string<hsd::char16>(42), u"42"));
    assert(equals(hsd::to_pseudo_string<hsd::char32>(42), U"42"));
}

static void check_wide_integer_limits()
{
    // 128-bit values fall outside printf, so their text is written out.
    const hsd::u128 _all_ones = ~static_cast<hsd::u128>(0);
    const hsd::i128 _min_i128 = static_cast<hsd::i128>(static_cast<hsd::u128>(1) << 127);

    assert(equals(hsd::to_pseudo_string(_all_ones), "340282366920938463463374607431768211455"));
    assert(equals(hsd::to_pseudo_string(_min_i128),
        "-170141183460469231731687303715884105728"));
    assert(equals(hsd::to_pseudo_string_hex(_all_ones), "0xffffffffffffffffffffffffffffffff"));
}

// Constant evaluation of the decimal and hexadecimal conversions.
static_assert(equals(hsd::to_pseudo_string(0), "0"));
static_assert(equals(hsd::to_pseudo_string(-17), "-17"));
static_assert(equals(hsd::to_pseudo_string(static_cast<hsd::i64>(-9223372036854775807LL - 1)),
    "-9223372036854775808"));
static_assert(equals(hsd::to_pseudo_string(static_cast<hsd::u64>(~0ull)),
    "18446744073709551615"));
static_assert(equals(hsd::to_pseudo_string_hex(static_cast<hsd::u8>(255)), "0xff"));
static_assert(equals(hsd::to_pseudo_string_hex(-1), "0xffffffff"));

int main()
{
    std::mt19937_64 rng{0x5EED'0003u};

    check_integer_type<hsd::i8>(rng);
    check_integer_type<hsd::u8>(rng);
    check_integer_type<hsd::i16>(rng);
    check_integer_type<hsd::u16>(rng);
    check_integer_type<hsd::i32>(rng);
    check_integer_type<hsd::u32>(rng);
    check_integer_type<hsd::i64>(rng);
    check_integer_type<hsd::u64>(rng);
    check_integer_type<hsd::isize>(rng);
    check_integer_type<hsd::usize>(rng);

    check_wide_integers();
    check_wide_integer_limits();

    return 0;
}
