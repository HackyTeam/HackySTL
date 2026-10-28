#include <Base/Utility.hpp>
#include <bit>
#include <cassert>

static_assert(hsd::countl_zero(static_cast<hsd::u8>(0)) == 8);
static_assert(hsd::countl_zero(static_cast<hsd::u8>(1)) == 7);
static_assert(hsd::countl_zero(static_cast<hsd::u16>(0x8000)) == 0);
static_assert(hsd::countl_zero(static_cast<hsd::u32>(1)) == 31);
static_assert(hsd::countl_zero(static_cast<hsd::u64>(1)) == 63);
static_assert(hsd::countl_zero(static_cast<hsd::u64>(0)) == 64);

static_assert(hsd::countr_zero(static_cast<hsd::u16>(0x8000)) == 15);
static_assert(hsd::countr_zero(static_cast<hsd::u8>(0)) == 8);

template <typename T>
static void check_value(T value)
{
    assert(hsd::countl_zero(value) == std::countl_zero(value));
    assert(hsd::countr_zero(value) == std::countr_zero(value));
}

template <typename T>
static void check_width()
{
    constexpr hsd::i32 digits = static_cast<hsd::i32>(sizeof(T) * 8);

    // Powers of two and their neighbours cover the boundary between every pair
    // of bit positions, including the most significant bit.
    for (hsd::i32 shift = 0; shift < digits; ++shift)
    {
        const T power = static_cast<T>(T{1} << shift);

        check_value(power);
        check_value(static_cast<T>(power - 1));
        check_value(static_cast<T>(power | 1));
        check_value(static_cast<T>(~power));
    }

    check_value(static_cast<T>(0));
    check_value(static_cast<T>(~T{0}));

    // A linear congruential sequence supplies values with mixed bit patterns.
    hsd::u64 seed = 0x9E37'79B9'7F4A'7C15ull;
    for (int index = 0; index < 10'000; ++index)
    {
        seed = seed * 6364136223846793005ull + 1442695040888963407ull;
        check_value(static_cast<T>(seed >> 32));
    }
}

int main()
{
    check_width<hsd::u8>();
    check_width<hsd::u16>();
    check_width<hsd::u32>();
    check_width<hsd::u64>();

    return 0;
}
