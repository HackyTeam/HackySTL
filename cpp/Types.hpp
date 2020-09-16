#pragma once

#include "_Define.hpp"

namespace hsd
{
    using usize = unsigned long;
    using isize = long;

    #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
    using u128 = __uint128_t;
    using i128 = __int128_t;
    #endif

    using u64 = unsigned long long;
    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = unsigned char;

    using i64 = long long;
    using i32 = int;
    using i16 = short;
    using i8 = char;

    using uchar = unsigned char;
    using wchar = wchar_t;
    using char16 = char16_t;
    using char32 = char32_t;

    using f128 = long double;
    using f64 = double;
    using f32 = float;
    
    using null = decltype(nullptr);
} // namespace hsd
