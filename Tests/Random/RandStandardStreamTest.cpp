#include <cassert>
#include <cmath>
#include <random>

#include <Base/Random.hpp>

// Regression test: bounded draws must come from the standard MT19937 stream. The second and
// third tempering steps shift left, as in the reference algorithm. A right shift there gave
// values that stayed in range but did not match std::mt19937.
int main()
{
    // Floating-point draws in [0, 1) are the raw word divided by 2^32 or 2^64.
    hsd::mt19937_32 _unit32;
    std::mt19937 _reference32;
    for (int _count = 0; _count < 2000; ++_count)
    {
        const double _expected = static_cast<double>(_reference32()) / 4294967296.0;
        const hsd::f64 _actual = _unit32.generate<hsd::f64>();
        assert(_actual == _expected);
    }

    hsd::mt19937_64 _unit64;
    std::mt19937_64 _reference64;
    for (int _count = 0; _count < 2000; ++_count)
    {
        const double _expected = static_cast<double>(_reference64()) / 18446744073709551616.0;
        const hsd::f64 _actual = _unit64.generate<hsd::f64>();
        assert(_actual == _expected);
    }

    // Integer ranges: generate(1, 7) scales the raw word the same way, then rounds down.
    hsd::mt19937_32 _dice;
    std::mt19937 _dice_reference;
    for (int _count = 0; _count < 2000; ++_count)
    {
        const double _multiplier = static_cast<double>(_dice_reference()) / (4294967295.0 + 1.0);
        const double _scaled = (7.0 - 1.0) * _multiplier + 1.0;
        const auto _expected = static_cast<hsd::i32>(std::floor(_scaled));
        const hsd::i32 _actual = _dice.generate(1, 7).unwrap();
        assert(_actual == _expected);
    }

    return 0;
}
