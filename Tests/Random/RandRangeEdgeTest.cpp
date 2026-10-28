#include <cassert>

#include <Base/Random.hpp>

// Bounds checks for ranged draws: every value lies in [from, to), for negative
// integer ranges and float ranges. The upper edge is handled by construction in
// `generate(from, to)`; the rounding case that reaches `to` occurs roughly once in
// 3e7 draws, so sampling cannot reliably reproduce it.
int main()
{
    hsd::mt19937_32 engine{2024};

    // Integers in a range that crosses zero.
    bool saw_first = false;
    bool saw_last = false;
    for (int draw = 0; draw < 20000; ++draw)
    {
        const hsd::i32 value = engine.generate<hsd::i32>(-5, 0).unwrap();
        assert(value >= -5 && value < 0);
        saw_first = saw_first || value == -5;
        saw_last = saw_last || value == -1;
    }
    assert(saw_first && saw_last);

    for (int draw = 0; draw < 20000; ++draw)
    {
        const hsd::i32 value = engine.generate<hsd::i32>(-100, 100).unwrap();
        assert(value >= -100 && value < 100);
    }

    for (int draw = 0; draw < 20000; ++draw)
    {
        const hsd::u32 value = engine.generate<hsd::u32>(0, 10).unwrap();
        assert(value < 10);
    }

    // Floats stay below the upper bound, even when it is a whole number.
    for (int draw = 0; draw < 20000; ++draw)
    {
        const hsd::f32 unit = engine.generate<hsd::f32>(0.0f, 1.0f).unwrap();
        assert(unit >= 0.0f && unit < 1.0f);

        const hsd::f64 wide = engine.generate<hsd::f64>(-1.0, 1.0).unwrap();
        assert(wide >= -1.0 && wide < 1.0);
    }

    assert(!engine.generate<hsd::i32>(3, 3).is_ok());
    assert(!engine.generate<hsd::f32>(2.0f, 1.0f).is_ok());
}
