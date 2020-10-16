#include "../../cpp/Io.hpp"

// custom struct for printing
struct test 
{
    int a = 1, b = 0;
    const char* c = "sample text";
    float d = 4.2;

    test() = default;

    test(int first, int second, const char* third, float forth)
        : a{first}, b{second}, c{third}, d{forth}
    {}
};

void _print(hsd::u8string& str, test& t, FILE* ptr = stdout)
{
    hsd::io::print<"{}Struct Test contains:\n[\n\ta = {}\n\tb = {}\n\tc = {}\n\td = {}\n]">(str, t.a, t.b, t.c, t.d);
}

void _print(hsd::wstring& str, test& t, FILE* ptr = stdout)
{
    hsd::io::wprint<L"{}Struct Test contains:\n[\n\ta = {}\n\tb = {}\n\tc = {}\n\td = {}\n]">(str, t.a, t.b, t.c, t.d);
}

void _parse(hsd::u8string& str, test& t)
{
    sscanf(str.c_str(), "%d%d%f", &t.a, &t.b, &t.d);
}

int main()
{
    int x, z;
    float y;
    hsd::io::print<"hello, {} and other words\n">(123.2f);
    hsd::io::print<"{}\n">(test{});
    hsd::io::print<"{}\n">(test{21, 1, "how is this possible", 69.420});
    auto t = hsd::io::read_line().parse<test>();
    hsd::io::read_line().set_data(x, y, z);
    hsd::io::print<"{}, {}, {}\n">(x, y, z);
    auto file = hsd::file("test.txt", hsd::file::options::text::read);
    auto b = file.read().parse<int>();
    auto e = file.read_line().to_string();
    hsd::io::print<"{}\n">(b);
}