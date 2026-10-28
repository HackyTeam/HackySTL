#include <cassert>

#include <Containers/StringView.hpp>

// Regression test: the single-character reverse search works in constant
// evaluation and on empty views, without stepping before the first character.
using namespace hsd::string_view_literals;

static_assert("a/b/c"_sv.rfind('/') == 3);
static_assert("abc"_sv.rfind('z') == hsd::string_view::npos);
static_assert(hsd::string_view{}.rfind('a') == hsd::string_view::npos);
static_assert("aaa"_sv.rfind('a') == 2);

int main()
{
    const hsd::string_view path = "dir/sub/file.txt";
    assert(path.rfind('/') == 7);
    assert(path.rfind('.') == 12);
    assert(path.rfind('q') == hsd::string_view::npos);

    const hsd::string_view empty{};
    assert(empty.rfind('x') == hsd::string_view::npos);
}
