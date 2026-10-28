#include <cassert>
#include <Containers/Integer.hpp>
#include <Threading/Time.hpp>

template <hsd::usize N>
static hsd::pair<hsd::integer<N>, hsd::integer<N>> bitwise_divide_reference(
    const hsd::integer<N>& numerator,
    const hsd::integer<N>& divisor)
{
    hsd::integer<N> quotient;
    hsd::integer<N> remainder;

    for (hsd::usize bit = N; bit > 0; --bit)
    {
        remainder <<= 1;
        quotient <<= 1;
        remainder.set(0, numerator.test(bit - 1).unwrap()).unwrap();
        if (remainder >= divisor)
        {
            remainder -= divisor;
            quotient.set(0).unwrap();
        }
    }

    return {quotient, remainder};
}

template <hsd::usize N>
static void test_reciprocal_division_width()
{
    hsd::integer<N> high_bit;
    high_bit.set(N - 1).unwrap();

    hsd::integer<N> patterned;
    hsd::u64 state = 0xA076'1D64'78BD'642FULL;
    for (hsd::usize bit = 0; bit < N; ++bit)
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        if ((state & 1) != 0)
        {
            patterned.set(bit).unwrap();
        }
    }

    const hsd::integer<N> numerators[] = {
        hsd::integer<N>{0}, high_bit, patterned
    };
    constexpr hsd::u32 divisors[] = {
        1, 2, 3, 65'535, 0x8000'0000U, 0xFFFF'FFFBu
    };

    for (const auto& numerator : numerators)
    {
        for (const hsd::u32 divisor : divisors)
        {
            const auto expected = bitwise_divide_reference(
                numerator, hsd::integer<N>{divisor});
            const auto actual = numerator.divide_with_reminder(
                hsd::integer<N>{divisor}).unwrap();
            assert(actual.quotient == expected.first);
            assert(actual.reminder == expected.second);
        }
    }
}

static void test_single_word_divisors()
{
    constexpr hsd::u64 numerators[] = {
        0,
        1,
        0xFFFF'FFFFULL,
        0x1'0000'0000ULL,
        0x1234'5678'9ABC'DEF0ULL,
        0xFFFF'FFFF'FFFF'FFFFULL
    };
    constexpr hsd::u32 divisors[] = {
        1,
        2,
        3,
        65'535,
        0x8000'0000U,
        0xFFFF'FFFFU
    };

    for (const hsd::u64 numerator : numerators)
    {
        for (const hsd::u32 divisor : divisors)
        {
            auto [quotient, remainder] = hsd::integer<64>{numerator}
                .divide_with_reminder(hsd::integer<64>{divisor}).unwrap();
            assert(quotient == hsd::integer<64>{numerator / divisor});
            assert(remainder == hsd::integer<64>{numerator % divisor});
        }
    }

    hsd::u64 state = 0xD1B5'4A32'D192'ED03ULL;
    for (hsd::usize iteration = 0; iteration < 256; ++iteration)
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        const hsd::u64 numerator = state;
        const hsd::u32 divisor = static_cast<hsd::u32>(state >> 32) | 1U;

        auto [quotient, remainder] = hsd::integer<64>{numerator}
            .divide_with_reminder(hsd::integer<64>{divisor}).unwrap();
        assert(quotient == hsd::integer<64>{numerator / divisor});
        assert(remainder == hsd::integer<64>{numerator % divisor});
    }

    hsd::integer<64> maximum = 0xFFFF'FFFF'FFFF'FFFFULL;
    hsd::integer<64> word_maximum = 0xFFFF'FFFFULL;
    auto [quotient, remainder] =
        maximum.divide_with_reminder(word_maximum).unwrap();
    assert(quotient == hsd::integer<64>{0x1'0000'0001ULL});
    assert(remainder == hsd::integer<64>{0});

    hsd::integer<65> partial_width = 0;
    partial_width.set(64).unwrap();
    partial_width.set(0).unwrap();
    hsd::integer<65> divisor = 0xFFFF'FFFFULL;
    auto [partial_quotient, partial_remainder] =
        partial_width.divide_with_reminder(divisor).unwrap();
    assert(partial_quotient == hsd::integer<65>{0x1'0000'0001ULL});
    assert(partial_remainder == hsd::integer<65>{2});

    hsd::integer<64> smaller = 17;
    auto [zero_quotient, unchanged_remainder] =
        smaller.divide_with_reminder(word_maximum).unwrap();
    assert(zero_quotient == hsd::integer<64>{0});
    assert(unchanged_remainder == smaller);

    auto [exact_quotient, zero_remainder] =
        hsd::integer<64>{123456}.divide_with_reminder(hsd::integer<64>{1}).unwrap();
    assert(exact_quotient == hsd::integer<64>{123456});
    assert(zero_remainder == hsd::integer<64>{0});

    auto [wide_quotient, wide_remainder] =
        hsd::integer<64>{0x3'0000'0000ULL}
            .divide_with_reminder(hsd::integer<64>{0x1'0000'0001ULL})
            .unwrap();
    assert(wide_quotient == hsd::integer<64>{2});
    assert(wide_remainder == hsd::integer<64>{0xFFFF'FFFEULL});
}

int main()
{
    using namespace hsd::format_literals;

    test_single_word_divisors();
    test_reciprocal_division_width<257>();
    test_reciprocal_division_width<1024>();

    hsd::integer<32> b1 = 123'132;
    hsd::integer<32> b2 = 234'543;

    assert(b2 - b1 == hsd::integer<32>{111411});
    assert(b2 + b1 == hsd::integer<32>{357675});
    assert(b2 * b1 == hsd::integer<32>{28'879'748'676});

    auto [quot, rem] = b2.divide_with_reminder(b1).unwrap();

    assert(quot == hsd::integer<32>{1});
    assert(rem == hsd::integer<32>{111411});
    assert(b2 / b1 == hsd::integer<32>{1});
    assert(b2 % b1 == hsd::integer<32>{111411});

    hsd::integer<32> numerator = 1000;
    hsd::integer<32> denominator = 37;
    auto [wide_quotient, wide_remainder] =
        numerator.divide_with_reminder(denominator).unwrap();
    assert(wide_quotient == hsd::integer<32>{27});
    assert(wide_remainder == hsd::integer<32>{1});
    assert(!numerator.divide_with_reminder(hsd::integer<32>{0}).is_ok());

    hsd::integer<4096> b3 = 0;

    b3 -= 1;

    assert(b3 != hsd::integer<4096>{0});
    assert(b3 + hsd::integer<4096>{1} == hsd::integer<4096>{0});
}