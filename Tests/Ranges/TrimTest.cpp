#include <Io.hpp>
#include <String.hpp>
#include <StringView.hpp>
#include <Range.hpp>

hsd::string trim(hsd::string_view const in)
{
    hsd::string str;
    auto rev_view = in
        | hsd::views::drop_while(hsd::cstring::iswhitespace)
        | hsd::views::reverse
        | hsd::views::drop_while(hsd::cstring::iswhitespace);

    for (auto& val : rev_view | hsd::views::reverse)
    {
        str.push_back(val);
    }

    return str;
}
 
int main()
{
    using namespace hsd::format_literals;

    const auto s = trim(" \f\n\t\r\vHello, C++20!\f\n\t\r\v ");
    hsd::println("\"{}\""_fmt, s);
 
    static constexpr hsd::stack_array v = {0, 1, 2, 3, 4, 5};
    
    for (auto n : v | hsd::views::drop_while([](hsd::i32 i) { return i < 3; }))
    {
        hsd::print("{} "_fmt, n);
    }

    hsd::print("\n"_fmt);
}