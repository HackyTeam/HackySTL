// Title: String view basics
// Level: Basic
// Shows: hsd::string_view as a non-owning window: find, rfind, prefix and suffix tests, sub_string, and constexpr use.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::string_view_literals;
using namespace hsd::format_literals;

// A view is a pointer and a length, so it can be built and checked at compile time.
constexpr hsd::string_view path_view = "docs/guide/intro.txt"_sv;
static_assert(path_view.length() == 20);
static_assert(path_view.find('/') == 4);
static_assert(path_view.ends_with(".txt"));

const char* yes_no(bool value)
{
    return value ? "true" : "false";
}

int main()
{
    hsd::string_view path = "docs/guide/intro.txt"_sv;
    hsd::println("path='{}' length={}"_fmt, path, path.length());

    // find() looks from the front, rfind() from the back; npos means no match.
    hsd::usize first_slash = path.find('/');
    hsd::usize last_slash = path.rfind('/');
    hsd::println("first slash at {}, last slash at {}"_fmt, first_slash, last_slash);
    hsd::println("find '#' is npos: {}"_fmt, yes_no(path.find('#') == hsd::string_view::npos));

    hsd::println("starts with docs: {}"_fmt, yes_no(path.starts_with("docs")));
    hsd::println("ends with .md: {}"_fmt, yes_no(path.ends_with(".md")));
    hsd::println("contains guide: {}"_fmt, yes_no(path.contains("guide")));

    // sub_string() returns a result holding another view onto the same text.
    // It is clipped at the end, and an out-of-range start is an error.
    auto directory = path.sub_string(0, last_slash);
    auto file_name = path.sub_string(last_slash + 1, hsd::string_view::npos);
    if (directory.is_ok() && file_name.is_ok())
    {
        hsd::println("dir: '{}' file: '{}'"_fmt, directory.unwrap(), file_name.unwrap());
    }
    hsd::println("sub_string(50, 1) is {}"_fmt, path.sub_string(50, 1).is_ok() ? "ok" : "an error");

    // find_first_of() stops at the first character from a set.
    hsd::println("first of '._' at {}"_fmt, path.find_first_of("._"));

    // Views compare by content, not by address.
    hsd::string owned{"docs/guide/intro.txt"};
    hsd::string_view over_owned{owned.c_str(), owned.length()};
    hsd::println("equal to the literal: {}"_fmt, yes_no(over_owned == path));
    hsd::println("same address: {}"_fmt, yes_no(over_owned.data() == path.data()));

    // A view sees later edits to the string it looks at, because it copies nothing.
    owned[0] = 'D';
    hsd::println("view now reads '{}'"_fmt, over_owned);

    // The reverse search works in constant expressions, and an empty view prints
    // as nothing without reading any memory.
    constexpr hsd::string_view slashes{"a/b/c"};
    static_assert(slashes.rfind('/') == 3);
    const hsd::string_view nothing{};
    hsd::println("[{}] rfind on empty: {}"_fmt, nothing,
        nothing.rfind('x') == hsd::string_view::npos ? "npos" : "found");
    return 0;
}
