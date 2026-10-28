#include <Io.hpp>
#include "../Headers/FpFormatter.hpp"

[[maybe_unused]] static constexpr void test_func();

int main()
{
    constexpr auto val = hsd::limits<hsd::f32>::max;
    using namespace hsd::format_literals;
    hsd::io::cout().get_stream().reserve(4096);
    auto string_rep = test::to_string(val);
    hsd::println("{}"_fmt, string_rep.data.data());
    
    return 0;
}

[[maybe_unused]] void test_fp()
{
    //hsd::f32 fp = 8388607.5f;
    //hsd::f64 fp = 4503599627370495.5;
    //hsd::f128 fp = 9223372036854775807.5l;

    //long l = 9223372036854775808l;

    /*for (; fp != std::floor(fp); fp += 1)
        ;

    hsd::println("{}"_fmt, fp);
    */
}


static constexpr void test_func()
{
    hsd::mt19937_64 twst;

    for (hsd::i32 i = 1; i < 16384; ++i)
    {
        hsd::f128 round = twst.generate(1ull, hsd::limits<hsd::u64>::max >> 16).unwrap();
        hsd::f128 point = 1. / twst.generate<hsd::u16>(1, 1023).unwrap();

        using namespace hsd::format_literals;
        auto string_rep = test::to_string(round + point);
        hsd::println("{} -> {}"_fmt, round + point, string_rep.data.data());
    }
}