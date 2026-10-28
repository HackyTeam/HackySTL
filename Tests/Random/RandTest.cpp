#include <cassert>
#include <random>

#include <Base/Random.hpp>

static void test_ranges()
{
    hsd::mt19937_32 engine{12345};
    bool saw_low = false;
    bool saw_high = false;

    for (int count = 0; count < 10000; ++count)
    {
        auto value = engine.generate(10u, 20u);
        assert(value.is_ok());
        const auto generated = value.unwrap();
        assert(generated >= 10u && generated < 20u);
        saw_low |= generated == 10u;
        saw_high |= generated == 19u;
    }

    assert(saw_low);
    assert(saw_high);
    assert(!engine.generate(4u, 4u).is_ok());
    assert(!engine.generate(5u, 4u).is_ok());

    for (int count = 0; count < 1000; ++count)
    {
        auto value = engine.generate(1.0, 4.0);
        assert(value.is_ok());
        const auto generated = value.unwrap();
        assert(generated >= 1.0 && generated < 4.0);
    }
}

static void test_reproducibility()
{
    hsd::mt19937_32 first{5489u};
    hsd::mt19937_32 second{5489u};

    for (int index = 0; index < 1000; ++index)
    {
        assert(first.generate() == second.generate());
    }

    hsd::mt19937_64 engine;
    engine.discard(9999);
    assert(engine.generate() == 9'981'545'732'273'789'042ull);

    hsd::mt19937_32 hacky_engine{12345u};
    std::mt19937 standard_engine{12345u};
    for (int index = 0; index < 1000; ++index)
    {
        assert(hacky_engine.generate() == standard_engine());
    }
}

int main()
{
    test_ranges();
    test_reproducibility();
}
