#include <Io.hpp>

// custom struct for printing
struct test 
{
    hsd::i32 a = 1, b = 0;
    char c[30] = "sample text";
    hsd::f64 d = 4.2;

    test() = default;

    test(hsd::i32 first, hsd::i32 second, const char* third, hsd::f64 forth)
        : a{first}, b{second}, d{forth}
    {
        hsd::cstring::copy(c, third);
    }
};

template <hsd::basic_string_literal str>
requires (hsd::IsSame<char, typename decltype(str)::char_type>)
inline void _print(test& t, FILE* ptr = stdout)
{
    if (ptr == stdout)
    {
        hsd::io_detail::_print<str>();
        hsd_print("Struct Test contains:\n[\n    a = {}\n    b = {}\n    c = {}\n    d = {}\n]", t.a, t.b, t.c, t.d);
    }
    else if (ptr == stderr)
    {
        hsd::io_detail::_print<str>(stderr);
        hsd_print_err("Struct Test contains:\n[\n    a = {}\n    b = {}\n    c = {}\n    d = {}\n]", t.a, t.b, t.c, t.d);
    }
}

template <hsd::basic_string_literal str>
requires (hsd::IsSame<hsd::wchar, typename decltype(str)::char_type>)
inline void _print(test& t, FILE* ptr = stdout)
{
    if (ptr == stdout)
    {
        // Optional: You can use hsd_print(L"{}", str) as well
        hsd::io_detail::_print<str>();
        hsd_print(L"Struct Test contains:\n[\n    a = {}\n    b = {}\n    c = {}\n    d = {}\n]", t.a, t.b, t.c, t.d);
    }
    else if (ptr == stderr)
    {
        hsd::io_detail::_print<str>(stderr);
        hsd_print_err(L"Struct Test contains:\n[\n    a = {}\n    b = {}\n    c = {}\n    d = {}\n]", t.a, t.b, t.c, t.d);
    }
}

inline void _parse(hsd::pair<const char*, hsd::usize>& str, test& t)
{
    sscanf(str.first, "%d%d%lf", &t.a, &t.b, &t.d);
}

int main()
{
    hsd::wsstream str{256};
    hsd::i32 x, z;
    hsd::f32 y;
    hsd_println(L"hello, {} and other words", 123.2);
    str.write_data<L"hello, {} and other words\n">(hsd::string("123.2"));
    
    hsd_println_err(L"{}", test{});
    hsd_println(L"{}", test{21, 1, "how is this possible", 69.42342});
    [[maybe_unused]] auto t = hsd::io::cin().read_line().unwrap().parse<test>();
    
    hsd::io::cin().read_line().unwrap().set_data(x, y, z).unwrap();
    hsd_println(L"{}, {}, {}", x, y, z);
    
    auto file = hsd::io::load_file(
        "Tests/Io/test.txt", hsd::io::options::text::read_write
    ).unwrap();
    
    [[maybe_unused]] auto c = file.read_line().unwrap().parse<hsd::i32>();
    file.print<"{} {} {}">("Hello", 12, -3.4).unwrap();
}