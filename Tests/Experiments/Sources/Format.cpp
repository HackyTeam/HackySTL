#include "../Headers/Print.hpp"

constexpr void print_test(auto&& value)
{
    constexpr hsd_test::fmt_common<char> fmt = {
        .tag = hsd_test::fmt_common<char>::none,
        .foreground = 0,
        .background = 0
    };

    if constexpr(requires {hsd_test::format<char, fmt>(value);})
    {
        auto res = hsd_test::format<char, fmt>(value);
        write(1, res.get_cstr(), res.len);
        write(1, "\n", 1);
    }
    else
    {
        write(1, value, sizeof(value));
    }
}

int main()
{
    constexpr auto value = 0x123456789ABCDEF0;
    print_test(value);

    return 0;
}