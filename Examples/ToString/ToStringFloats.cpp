// Title: Float formatting
// Level: Intermediate
// Shows: fixed, scientific and hexadecimal float text, precision, rounding and special values.

#include <Base/Limits.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/ToString.hpp>

using namespace hsd::format_literals;

template <typename Buffer>
static void show(const char* label, const Buffer& text)
{
    hsd::string_view view{text.data(), text.size()};
    hsd::println("{}: {}"_fmt, label, view);
}

int main()
{
    // Fixed notation has six digits after the point unless a precision is given.
    show("default", hsd::to_pseudo_string(3.14159265));
    show("precision 2", hsd::to_pseudo_string<char, 2>(3.14159265));
    show("precision 0", hsd::to_pseudo_string<char, 0>(7.5));

    // Ties go to the even neighbour, like printf: 0.5 and 2.5 round down, 3.5 up.
    show("0.5 -> 0", hsd::to_pseudo_string<char, 0>(0.5));
    show("2.5 -> 2", hsd::to_pseudo_string<char, 0>(2.5));
    show("3.5 -> 4", hsd::to_pseudo_string<char, 0>(3.5));

    // A carry can reach the integral part.
    show("carry", hsd::to_pseudo_string(9.9999996));

    // Scientific notation has one digit before the point.
    show("exp", hsd::to_pseudo_string_exp(12345.678));
    show("exp small", hsd::to_pseudo_string_exp(-0.000123456789));
    show("exp prec 2", hsd::to_pseudo_string_exp<char, 2>(6.02214e23));

    // The hexadecimal form is exact: a power of two has no fraction digits.
    show("hex 1.0", hsd::to_pseudo_string_hex(1.0));
    show("hex 0.375", hsd::to_pseudo_string_hex(0.375));
    show("hex -2.5", hsd::to_pseudo_string_hex(-2.5));

    // Special values have fixed spellings.
    show("infinity", hsd::to_pseudo_string(hsd::limits<hsd::f64>::infinity));
    show("-infinity", hsd::to_pseudo_string(-hsd::limits<hsd::f64>::infinity));

    return 0;
}
