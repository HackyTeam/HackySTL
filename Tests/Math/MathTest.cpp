#include "../../cpp/Math.hpp"
#include <stdio.h>

int main()
{
    constexpr hsd::f32 val1 = hsd::math::sqrt(9.f);
    hsd::f32 val2 = hsd::math::constexpr_math::mod(123.123123f, 21.f);
    printf("%f\n%f\n", val1, val2);
}