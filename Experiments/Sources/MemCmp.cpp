#include "Types.hpp"
#include <Utility.hpp>
#include <StackArray.hpp>

struct test
{
    hsd::i32 a;
    hsd::f32 b;
    char c[7];
    hsd::u64 d;
};

template <typename T, typename U>
static constexpr hsd::i32 mem_cmp(const T& src1, const U& src2)
{
    constexpr auto sz = sizeof(T) > sizeof(U) ? sizeof(U) : sizeof(T);

    const auto _bytes_src1 = hsd::bit_cast<hsd::stack_array<hsd::uchar, sizeof(T)>>(src1);
    const auto _bytes_src2 = hsd::bit_cast<hsd::stack_array<hsd::uchar, sizeof(U)>>(src2);
    
    hsd::usize _idx = 0;
    
    while (_idx != sz)
    {
        if (_bytes_src1[_idx] != _bytes_src2[_idx])
        {
            return _bytes_src1[_idx] - _bytes_src2[_idx];
        }

        ++_idx;
    }

    if constexpr (sizeof(T) > sizeof(U))
    {
        return _bytes_src1[_idx];
    }
    else if constexpr (sizeof(T) < sizeof(U))
    {
        return _bytes_src2[_idx];
    }

    return 0;
}

int main()
{
    constexpr test v1 = {
        .a = -6,
        .b = 2143.2312,
        .c = "yo wtf",
        .d = 123
    };

    constexpr test v2 = {
        .a = -6,
        .b = 2143.2312,
        .c = "yo wtf",
        .d = 123
    };

    constexpr auto res = mem_cmp(v1, v2);

    return res;
}