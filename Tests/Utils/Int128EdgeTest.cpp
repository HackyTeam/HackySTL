#include <Base/_Define.hpp>
#include <Base/_Int128.hpp>
#include <cassert>

// Shifts by zero and by 128 or more, signed comparisons whose high words are equal, and
// division. Division is checked against the compiler's native 128-bit integers on edge
// values and on seeded random operands, where the compiler has __int128.

using lib_u128 = hsd::int128_detail::basic_int128<false>;
using lib_i128 = hsd::int128_detail::basic_int128<true>;

namespace
{
    // Reads the value through a volatile object, so the call is made at run time and is not
    // folded into a constant. The coverage counters only see code that runs.
    template <typename T>
    T runtime_value(T value)
    {
        volatile T _stored = value;
        return _stored;
    }

    void check_shift_edges()
    {
        const lib_u128 _all{~0ull, ~0ull};
        assert((lib_u128{5u} << lib_u128{0u}) == lib_u128{5u});
        assert((_all >> lib_u128{0u}) == _all);
        assert((_all >> lib_u128{128u}) == lib_u128{});
        assert((_all >> lib_u128{200u}) == lib_u128{});

        // Arithmetic shifts keep the sign: a count of 128 or more leaves -1 or 0.
        const lib_i128 _negative{static_cast<long long>(-5)};
        const lib_i128 _positive{5};
        assert((_negative >> lib_i128{0}) == _negative);
        assert((_negative >> lib_i128{3}) == lib_i128{static_cast<long long>(-1)});
        assert((_negative >> lib_i128{128}) == lib_i128{static_cast<long long>(-1)});
        assert((_negative >> lib_i128{200}) == lib_i128{static_cast<long long>(-1)});
        assert((_positive >> lib_i128{128}) == lib_i128{});
        assert((_positive << lib_i128{0}) == _positive);

        // Left shifts by 64 to 127 move bits into the high word, and by 128 or more give zero.
        assert(((lib_u128{5u} << lib_u128{64u}) == lib_u128{0, 5}));
        assert(((lib_u128{5u} << lib_u128{100u}) == lib_u128{0, 5ull << 36}));
        assert((lib_u128{runtime_value(5u)} << lib_u128{runtime_value(128u)}) == lib_u128{});
    }

    void check_constructor_and_zero_edges()
    {
        // An unsigned value built from a negative integer has every high bit set.
        assert((lib_u128{runtime_value(-1)}) == (lib_u128{~0ull, ~0ull}));
        assert(hsd::int128_detail::basic_int128<false>::leading_zeros(runtime_value(0ull)) == 64);
        assert(hsd::int128_detail::basic_int128<false>::leading_zeros(1) == 63);
    }

    void check_signed_compare_edges()
    {
        // Equal high words compare by the low words, for non-negative and negative values.
        assert((lib_i128{1ull, 0ull} < lib_i128{2ull, 0ull}));
        assert((lib_i128{2ull, 0ull} <= lib_i128{2ull, 0ull}));
        assert((lib_i128{1ull, ~0ull} < lib_i128{2ull, ~0ull}));
        assert(!(lib_i128{2ull, ~0ull} < lib_i128{1ull, ~0ull}));

        // Values with different high words compare by the high words, as signed numbers.
        assert((lib_i128{0ull, 1ull} < lib_i128{0ull, 2ull}));
        assert((lib_i128{0ull, ~0ull} < lib_i128{0ull, 1ull}));
        assert((lib_i128{7ull, ~0ull} < lib_i128{0ull, 1ull}));
    }

#if defined(__SIZEOF_INT128__)
    unsigned __int128 native_unsigned(lib_u128 value)
    {
        return (static_cast<unsigned __int128>(value._hi) << 64) | value._lo;
    }

    __int128 native_signed(lib_i128 value)
    {
        return static_cast<__int128>(native_unsigned(lib_u128{value._lo, value._hi}));
    }

    lib_i128 lib_signed(__int128 value)
    {
        const auto _bits = static_cast<unsigned __int128>(value);
        return lib_i128{static_cast<unsigned long long>(_bits),
            static_cast<unsigned long long>(_bits >> 64)};
    }

    // Division by zero and the one overflowing quotient, the minimum value divided by -1,
    // are not defined for the native type, so they are not compared.
    bool signed_division_is_defined(__int128 dividend, __int128 divisor)
    {
        const __int128 _minimum = static_cast<__int128>(static_cast<unsigned __int128>(1) << 127);
        return divisor != 0 && !(dividend == _minimum && divisor == -1);
    }

    void check_division_against_native(lib_u128 dividend, lib_u128 divisor)
    {
        const unsigned __int128 _u = native_unsigned(dividend);
        const unsigned __int128 _v = native_unsigned(divisor);
        const lib_u128 _q = dividend / divisor;
        const lib_u128 _r = dividend % divisor;
        assert(native_unsigned(_q) == _u / _v);
        assert(native_unsigned(_r) == _u % _v);
    }

    void check_signed_division_against_native(__int128 dividend, __int128 divisor)
    {
        const lib_i128 _q = lib_signed(dividend) / lib_signed(divisor);
        const lib_i128 _r = lib_signed(dividend) % lib_signed(divisor);
        assert(native_signed(_q) == dividend / divisor);
        assert(native_signed(_r) == dividend % divisor);
    }

    // Words that give quotient digits near the limits of the word-by-word division.
    constexpr unsigned long long edge_words[] = {
        0, 1, 2, 3, 0x7fffffffffffffffull, 0x8000000000000000ull, 0x8000000000000001ull,
        0xfffffffffffffffeull, ~0ull, 0x100000000ull, 0xffffffffull, 0x0123456789abcdefull,
    };

    void check_edge_division()
    {
        for (const unsigned long long _uh : edge_words)
        {
            for (const unsigned long long _ul : edge_words)
            {
                for (const unsigned long long _vh : edge_words)
                {
                    for (const unsigned long long _vl : edge_words)
                    {
                        const lib_u128 _divisor{_vl, _vh};
                        if (_divisor == lib_u128{})
                        {
                            continue;
                        }

                        check_division_against_native(lib_u128{_ul, _uh}, _divisor);

                        const __int128 _dividend = static_cast<__int128>(native_unsigned(lib_u128{_ul, _uh}));
                        const __int128 _signed_divisor = static_cast<__int128>(native_unsigned(_divisor));
                        if (signed_division_is_defined(_dividend, _signed_divisor))
                        {
                            check_signed_division_against_native(_dividend, _signed_divisor);
                        }
                    }
                }
            }
        }
    }

    // The word-by-word division uses 32-bit digits. Operands whose digits sit at the
    // boundaries of the normalisation and correction steps give the adjusted quotients.
    void check_word_division_digits()
    {
        constexpr unsigned long long digits[] = {0, 1, 0x7fffffffull, 0x80000000ull, 0xffffffffull};
        unsigned long long _words[25]{};
        int _count = 0;
        for (const unsigned long long _high : digits)
        {
            for (const unsigned long long _low : digits)
            {
                _words[_count++] = (_high << 32) | _low;
            }
        }

        for (const unsigned long long _v : _words)
        {
            if (_v == 0)
            {
                continue;
            }

            for (const unsigned long long _u1 : _words)
            {
                if (_u1 >= _v)
                {
                    continue;
                }

                for (const unsigned long long _u0 : _words)
                {
                    unsigned long long _remainder = 0;
                    const unsigned long long _quotient =
                        hsd::int128_detail::basic_int128<false>::divide_2_by_1(_u1, _u0, _v, _remainder);
                    const unsigned __int128 _n = (static_cast<unsigned __int128>(_u1) << 64) | _u0;
                    assert(_quotient == static_cast<unsigned long long>(_n / _v));
                    assert(_remainder == static_cast<unsigned long long>(_n % _v));
                }
            }
        }
    }

    void check_random_division()
    {
        // A fixed xorshift sequence keeps the run the same on every platform.
        unsigned long long _state = 0x9e3779b97f4a7c15ull;
        auto next = [&_state]
        {
            _state ^= _state << 13;
            _state ^= _state >> 7;
            _state ^= _state << 17;
            return _state;
        };

        for (int _index = 0; _index < 50000; ++_index)
        {
            // Shifting by a random count gives divisors of very different sizes.
            const lib_u128 _dividend{next(), next()};
            const lib_u128 _divisor = lib_u128{next(), next()} >> lib_u128{next() % 128};
            if (_divisor == lib_u128{})
            {
                continue;
            }
            check_division_against_native(_dividend, _divisor);

            const lib_i128 _signed_dividend{next(), next()};
            const lib_i128 _signed_divisor = lib_i128{next(), next()} >> lib_i128{next() % 128};
            if (_signed_divisor == lib_i128{})
            {
                continue;
            }
            if (signed_division_is_defined(native_signed(_signed_dividend), native_signed(_signed_divisor)))
            {
                check_signed_division_against_native(native_signed(_signed_dividend),
                    native_signed(_signed_divisor));
            }
        }
    }
#endif
}

int main()
{
    check_shift_edges();
    check_signed_compare_edges();
    check_constructor_and_zero_edges();

#if defined(__SIZEOF_INT128__)
    check_edge_division();
    check_word_division_digits();
    check_random_division();
#endif
    return 0;
}
