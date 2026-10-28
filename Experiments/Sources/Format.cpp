#include "../Headers/Print.hpp"

constexpr void print_test(auto&& value)
{
    constexpr hsd_test::fmt_common<char> fmt = {
        .tag = hsd_test::fmt_common<char>::none,
        .foreground = 0,
        .background = 0
    };

    //if constexpr(requires {hsd_test::format<char, fmt>(value);})
    //{
        auto res = hsd_test::format<char, fmt>(value);
        write(1, res.get_cstr(), res.len);
        write(1, "\n", 1);
    /*}
    else if constexpr (hsd::is_array<decltype(value)>::value)
    {
        write(1, value, sizeof(value));
    }*/
}

constexpr auto to_dec(hsd::f128 value)
{
    struct dec_repres
    {
        hsd::uchar buf[16];
    };

    return hsd::bit_cast<dec_repres>(value);
}

int main()
{
    constexpr auto value = 0x123456789ABCDEF0;
    print_test(value);

    constexpr auto val2 = to_dec(0.99999994f);

    return 0;
}