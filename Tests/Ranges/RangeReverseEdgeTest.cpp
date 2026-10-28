#include <cassert>

#include <Containers/Range.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>

using namespace hsd::string_view_literals;

// Regression tests: reverse views accept string views, strings and vectors,
// and an empty source produces an empty view.
int main()
{
    hsd::u32 digits = 0;
    for (const char letter : "abc"_sv | hsd::views::reverse)
    {
        digits = digits * 10 + static_cast<hsd::u32>(letter - 'a' + 1);
    }
    assert(digits == 321);

    hsd::string text{"xyz"};
    hsd::u32 from_string = 0;
    for (const char letter : text | hsd::views::reverse)
    {
        from_string = from_string * 10 + static_cast<hsd::u32>(letter - 'x' + 1);
    }
    assert(from_string == 321);

    hsd::vector<hsd::i32> values{{1, 2, 3}};
    hsd::i32 from_vector = 0;
    for (const hsd::i32 value : values | hsd::views::reverse)
    {
        from_vector = from_vector * 10 + value;
    }
    assert(from_vector == 321);

    const hsd::string_view blank{};
    for (const char letter : blank | hsd::views::reverse)
    {
        (void)letter;
        assert(false);
    }
}
