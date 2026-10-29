#include <Base/_Define.hpp>
#include <Base/_Int128.hpp>
#include <cassert>

// The library's 128-bit integer is tested directly, so these checks run on every
// compiler. Formatting needs the library type to be the `u128` alias, which is the
// case on MSVC and in builds that define HSD_LIBRARY_INT128.
#if !defined(HSD_COMPILER_GCC) || defined(HSD_LIBRARY_INT128)
#define HSD_INT128_FORMATTING 1
#include <ToString/TextCheck.hpp>
#include <Serialize/ToString.hpp>
#endif

using lib_u128 = hsd::int128_detail::basic_int128<false>;
using lib_i128 = hsd::int128_detail::basic_int128<true>;

static_assert((lib_u128{1u} << 127) == lib_u128{0, 1ull << 63});
static_assert((lib_u128{0, 1} / lib_u128{10u}) == lib_u128{1844674407370955161ull});
static_assert((lib_u128{0, 1} % lib_u128{10u}) == lib_u128{6u});

static void check_unsigned_arithmetic()
{
    const lib_u128 _max{~0ull, ~0ull};
    const lib_u128 _two_64{0, 1};

    assert((_max + lib_u128{1u} == lib_u128{}));
    assert((lib_u128{~0ull, 0} + lib_u128{1u} == _two_64));
    assert((_two_64 - lib_u128{1u} == lib_u128{~0ull, 0}));

    // (2^64 - 1)^2 = 2^128 - 2^65 + 1
    assert((lib_u128{~0ull} * lib_u128{~0ull} == lib_u128{1u, ~0ull - 1}));

    // Divisors below 2^64 take the word path, and larger divisors the bit-by-bit path.
    const lib_u128 _n{0x123456789ABCDEF0ull, 0xFEDCBA9876543210ull};
    const lib_u128 _d{1u, 1u};
    const lib_u128 _q = _n / _d;
    const lib_u128 _r = _n % _d;
    assert((_q * _d + _r == _n));
    assert((_r < _d));

    const lib_u128 _big{0x0FEDCBA987654321ull, 0x1234ull};
    assert((_big / lib_u128{7u} * lib_u128{7u} + _big % lib_u128{7u} == _big));

    assert((lib_u128{0xF0u} & lib_u128{0x3Cu}) == lib_u128{0x30u});
    assert((lib_u128{0xF0u} | lib_u128{0x0Fu}) == lib_u128{0xFFu});
    assert((lib_u128{0xFFu} ^ lib_u128{0x0Fu}) == lib_u128{0xF0u});
    assert((~lib_u128{} == _max));
}

static void check_shifts_and_comparisons()
{
    assert(((lib_u128{1u} << 127) == lib_u128{0, 1ull << 63}));
    assert(((lib_u128{1u} << 127) >> 127) == lib_u128{1u});
    assert(((lib_u128{1u} << 128) == lib_u128{}));
    assert(((lib_u128{1u} << 64) == lib_u128{0, 1}));

    assert((lib_u128{0} < lib_u128{1u}));
    assert((lib_u128{~0ull, 0} < lib_u128{0, 1}));
    assert((lib_u128{0, 1} >= lib_u128{~0ull, 0}));
    assert((!(lib_u128{5u} == lib_u128{6u})));
}

static void check_signed_arithmetic()
{
    const lib_i128 _minus_one{-1};

    assert((_minus_one == lib_i128{~0ull, ~0ull}));
    assert((_minus_one >> 1) == _minus_one);
    assert((lib_i128{-7} / lib_i128{2} == lib_i128{-3}));
    assert((lib_i128{-7} % lib_i128{2} == lib_i128{-1}));
    assert((lib_i128{7} % lib_i128{-2} == lib_i128{1}));
    assert((lib_i128{-1} < lib_i128{0}));
    assert((-lib_i128{5} == lib_i128{-5}));
    assert((static_cast<lib_u128>(_minus_one) == lib_u128{~0ull, ~0ull}));
}

static void check_conversions()
{
    assert((static_cast<unsigned long long>(lib_u128{0x1234ull, 5ull}) == 0x1234ull));
    assert((static_cast<double>(lib_u128{0, 1}) == 18446744073709551616.0));

    // 1e20 = 5 * 2^64 + 0x6BC75E2D63100000, and both values are exact in a double.
    assert((static_cast<lib_u128>(1e20) == lib_u128{0x6BC75E2D63100000ull, 5ull}));
    assert((static_cast<double>(lib_u128{0x6BC75E2D63100000ull, 5ull}) == 1e20));

    // Conversion from a negative floating-point value truncates toward zero.
    assert((static_cast<lib_i128>(-2.5) == lib_i128{-2}));
}

#if HSD_INT128_FORMATTING
static void check_formatting()
{
    assert((equals(hsd::to_pseudo_string(lib_u128{0, 1}), "18446744073709551616")));
    assert((equals(hsd::to_pseudo_string(lib_i128{-5}), "-5")));
    assert(equals(hsd::to_pseudo_string_hex(lib_u128{~0ull, ~0ull}),
        "0xffffffffffffffffffffffffffffffff"));
}
#endif

int main()
{
    check_unsigned_arithmetic();
    check_shifts_and_comparisons();
    check_signed_arithmetic();
    check_conversions();
#if HSD_INT128_FORMATTING
    check_formatting();
#endif

    return 0;
}
