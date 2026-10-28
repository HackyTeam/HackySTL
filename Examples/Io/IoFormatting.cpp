// Title: Formatted output
// Level: Basic
// Shows: print and println with _fmt literals, {hex}, {prec=N}, {exp}, chars and strings.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

int main()
{
    // A "_fmt" literal is checked at compile time: every {} takes one argument.
    hsd::println("integers: {} {} {}"_fmt, 42, -17, 1000000);
    hsd::println("chars and text: {} {} {}"_fmt, 'A', "C string", "view"_sv);

    hsd::string owned = hsd::string{"owned"};
    hsd::println("owned string: {} (length {})"_fmt, owned, owned.length());

    // Tags go inside the braces.
    hsd::println("hex: {hex} {hex}"_fmt, 255, 4096);

    // Floats use six digits by default, {prec=N} chooses the digits after the
    // point, and {exp} switches to scientific notation.
    hsd::f64 pi = 3.14159265358979;
    hsd::println("default: {}"_fmt, pi);
    hsd::println("two digits: {prec=2}"_fmt, pi);
    hsd::println("zero digits: {prec=0}"_fmt, 2.5);
    hsd::println("scientific: {exp}"_fmt, 123456.789);
    hsd::println("small scientific: {exp}"_fmt, 0.00042);

    // print leaves the line open, so several calls can build one line.
    hsd::print("row:"_fmt);
    for (hsd::i32 value = 1; value <= 4; ++value)
    {
        hsd::print(" {}"_fmt, value * value);
    }
    hsd::println(""_fmt);

    return 0;
}
