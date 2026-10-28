#include <Base/Limits.hpp>
#include <cassert>
#include <limits>

int main()
{
    static_assert(hsd::limits<hsd::i8>::min == -128);
    static_assert(hsd::limits<hsd::i8>::min == std::numeric_limits<hsd::i8>::min());
    static_assert(hsd::limits<hsd::i8>::max == 127);
    static_assert(hsd::limits<hsd::i8>::max == std::numeric_limits<hsd::i8>::max());
    static_assert(hsd::limits<hsd::i16>::min < 0);
    static_assert(hsd::limits<hsd::i16>::max > 0);
    static_assert(hsd::limits<hsd::u8>::min == 0);
    static_assert(hsd::limits<hsd::u8>::max == 255);
    static_assert(hsd::limits<hsd::u8>::max == std::numeric_limits<hsd::u8>::max());
    static_assert(hsd::limits<hsd::u16>::min == 0);
    static_assert(hsd::limits<hsd::u16>::max == 65535);
    static_assert(hsd::limits<hsd::i32>::min < hsd::limits<hsd::i32>::max);
    static_assert(hsd::limits<hsd::u32>::min == 0);
    static_assert(hsd::limits<hsd::u32>::max == 4294967295U);
    static_assert(hsd::limits<hsd::u32>::max == std::numeric_limits<hsd::u32>::max());
    static_assert(hsd::limits<hsd::i64>::min < hsd::limits<hsd::i64>::max);
    static_assert(hsd::limits<hsd::u64>::min == 0);
    static_assert(hsd::limits<hsd::u64>::max > hsd::limits<hsd::u32>::max);
    static_assert(hsd::limits<hsd::i8>::is_signed);
    static_assert(hsd::limits<hsd::i8>::is_signed == std::numeric_limits<hsd::i8>::is_signed);
    static_assert(!hsd::limits<hsd::u8>::is_signed);
    static_assert(hsd::limits<hsd::f32>::is_specialized);
    static_assert(hsd::limits<hsd::f32>::is_specialized
        == std::numeric_limits<hsd::f32>::is_specialized);
    assert(hsd::limits<hsd::f32>::min > 0);
    assert(hsd::limits<hsd::f32>::min == std::numeric_limits<hsd::f32>::min());
    assert(hsd::limits<hsd::f64>::max > hsd::limits<hsd::f32>::max);
    assert(hsd::limits<hsd::f64>::max == std::numeric_limits<hsd::f64>::max());
    assert(hsd::limits<hsd::f32>::denorm_min > 0);
    assert(hsd::limits<hsd::f32>::denorm_min == std::numeric_limits<hsd::f32>::denorm_min());
    assert(hsd::limits<hsd::f64>::epsilon < hsd::limits<hsd::f32>::epsilon);
    assert(hsd::limits<hsd::f64>::epsilon == std::numeric_limits<hsd::f64>::epsilon());
}