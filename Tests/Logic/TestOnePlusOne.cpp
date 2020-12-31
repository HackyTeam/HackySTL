#include <Extra/Constexpr/BinaryArithmetic.hpp>

int main() 
{
    static_assert(hsd::is_same<hsd::ripple_adder<hsd::natural::one, hsd::natural::one, hsd::bit::zero>, hsd::natural::two>::value);
}
