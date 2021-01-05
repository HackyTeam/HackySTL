#include <Math.hpp>
#include <stdio.h>
#include <Io.hpp>

int main()
{
    constexpr hsd::f64 val1 = hsd::math::sqrt(5.0);
    hsd::f64 val2 = std::sqrt(5.0);
    hsd::io::print<"{} {}\n">(val1, val2);
}