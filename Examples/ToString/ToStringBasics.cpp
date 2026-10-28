// Title: Number to text basics
// Level: Basic
// Shows: hsd::to_string for owning strings and to_pseudo_string for fixed buffers.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/ToString.hpp>

using namespace hsd::format_literals;

// A pseudo string is a small fixed buffer with data() and size(), and no
// terminating NUL. A string_view over it can be printed.
template <typename Buffer>
static void show(const char* label, const Buffer& text)
{
    hsd::string_view view{text.data(), text.size()};
    hsd::println("{} -> {} (size {})"_fmt, label, view, text.size());
}

int main()
{
    // to_string allocates an hsd::string. Floats use six digits by default.
    hsd::string a = hsd::to_string(42);
    hsd::string b = hsd::to_string(-7);
    hsd::string c = hsd::to_string(2.5);
    hsd::println("{} {} {}"_fmt, a, b, c);

    // The length is the character count, without the terminator.
    hsd::println("lengths: {} {} {}"_fmt, a.length(), b.length(), c.length());

    // to_pseudo_string needs no allocation and also works at compile time.
    show("decimal 255", hsd::to_pseudo_string(255));
    show("negative", hsd::to_pseudo_string(-1024));
    show("hex 255", hsd::to_pseudo_string_hex(255));
    show("zero", hsd::to_pseudo_string(0));

    // to_wstring and its relatives build strings of other character types.
    hsd::wstring wide = hsd::to_wstring(2024);
    hsd::println("wide length: {}"_fmt, wide.length());

    return 0;
}
