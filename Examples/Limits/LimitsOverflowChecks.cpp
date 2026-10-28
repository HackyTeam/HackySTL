// Title: Overflow checks
// Level: Intermediate
// Shows: detecting integer overflow before it happens with limits, plus compile-time checks.

#include <Base/Limits.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Signed overflow is undefined behavior, so the test must run before the operation.
template <typename T>
constexpr bool add_overflows(T left, T right)
{
    if (right > 0)
    {
        return left > hsd::limits<T>::max - right;
    }
    if constexpr (hsd::limits<T>::is_signed)
    {
        return right < 0 && left < hsd::limits<T>::min - right;
    }
    else
    {
        return false;
    }
}

template <typename T>
constexpr bool mul_overflows(T left, T right)
{
    if (left == 0 || right == 0)
    {
        return false;
    }

    // Dividing the limit by one factor shows the largest the other may be.
    if (left > 0 && right > 0)
    {
        return left > hsd::limits<T>::max / right;
    }
    if constexpr (hsd::limits<T>::is_signed)
    {
        if (left < 0 && right < 0)
        {
            return left < hsd::limits<T>::max / right;
        }
        if (left > 0)
        {
            return right < hsd::limits<T>::min / left;
        }
        return left < hsd::limits<T>::min / right;
    }
    else
    {
        return false;
    }
}

static_assert(!add_overflows<hsd::i8>(100, 27));
static_assert(add_overflows<hsd::i8>(100, 28));
static_assert(add_overflows<hsd::i8>(-100, -29));
static_assert(!add_overflows<hsd::u8>(200, 55));
static_assert(add_overflows<hsd::u8>(200, 56));
static_assert(mul_overflows<hsd::i32>(65536, 32768));
static_assert(!mul_overflows<hsd::i32>(65536, 32767));

// How many digits does the largest value need? It sizes a text buffer.
template <typename T>
constexpr hsd::usize decimal_digits()
{
    hsd::usize digits = 1;
    for (T value = hsd::limits<T>::max; value >= 10; value /= 10)
    {
        ++digits;
    }
    return digits;
}

static_assert(decimal_digits<hsd::u8>() == 3);
static_assert(decimal_digits<hsd::u32>() == 10);

template <typename T>
void check_add(const char* type, T left, T right)
{
    // 8 bit integers are character types, so widen them to print numbers.
    hsd::println("{} {} + {}: {}"_fmt, type,
        static_cast<hsd::i64>(left), static_cast<hsd::i64>(right),
        add_overflows(left, right) ? "overflow" : "fits");
}

int main()
{
    check_add<hsd::i8>("i8", 100, 27);
    check_add<hsd::i8>("i8", 100, 28);
    check_add<hsd::i8>("i8", -100, -29);
    check_add<hsd::u8>("u8", 200, 55);
    check_add<hsd::u8>("u8", 200, 56);
    check_add<hsd::i32>("i32", 2147483000, 647);
    check_add<hsd::i32>("i32", 2147483000, 648);

    hsd::println("i32 65536 * 32767: {}"_fmt,
        mul_overflows<hsd::i32>(65536, 32767) ? "overflow" : "fits");
    hsd::println("i32 65536 * 32768: {}"_fmt,
        mul_overflows<hsd::i32>(65536, 32768) ? "overflow" : "fits");
    hsd::println("i32 -65536 * 32768: {}"_fmt,
        mul_overflows<hsd::i32>(-65536, 32768) ? "overflow" : "fits");
    hsd::println("i32 -65536 * 32769: {}"_fmt,
        mul_overflows<hsd::i32>(-65536, 32769) ? "overflow" : "fits");

    // Unsigned arithmetic is defined to wrap, which limits makes easy to see.
    hsd::u8 counter = hsd::limits<hsd::u8>::max;
    ++counter;
    hsd::println("u8 max + 1 wraps to {}"_fmt, static_cast<hsd::i32>(counter));

    // Factorials: find the largest n whose factorial still fits in 32 and 64 bits.
    hsd::u32 fits32 = 1;
    hsd::u32 fact32 = 1;
    while (!mul_overflows<hsd::u32>(fact32, fits32 + 1))
    {
        ++fits32;
        fact32 *= fits32;
    }

    hsd::u64 fits64 = 1;
    hsd::u64 fact64 = 1;
    while (!mul_overflows<hsd::u64>(fact64, fits64 + 1))
    {
        ++fits64;
        fact64 *= fits64;
    }

    hsd::println("largest factorial in u32: {}! = {}"_fmt, fits32, fact32);
    hsd::println("largest factorial in u64: {}! = {}"_fmt, fits64, fact64);
    hsd::println("digits of u32 max: {}, of u64 max: {}"_fmt,
        decimal_digits<hsd::u32>(), decimal_digits<hsd::u64>());

    return 0;
}
