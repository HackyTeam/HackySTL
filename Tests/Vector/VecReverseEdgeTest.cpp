#include <cassert>

#include <Containers/StringView.hpp>
#include <Containers/Range.hpp>
#include <Containers/Vector.hpp>

using namespace hsd::string_view_literals;

// Regression tests for reverse traversal: empty containers, constant evaluation
// of string-view reverse iterators, reverse views, and const access to fixed storage.
static_assert(*"abc"_sv.rbegin() == 'c');

int main()
{
    hsd::vector<hsd::i32> values{{1, 2, 3}};
    hsd::i32 reversed = 0;
    for (auto it = values.rbegin(); it != values.rend(); ++it)
    {
        reversed = reversed * 10 + *it;
    }
    assert(reversed == 321);

    hsd::vector<hsd::i32> empty;
    for (auto it = empty.rbegin(); it != empty.rend(); ++it)
    {
        assert(false);
    }
    assert(empty.rbegin() == empty.rend());

    hsd::i32 from_view = 0;
    for (auto& value : values | hsd::views::reverse)
    {
        from_view = from_view * 10 + value;
    }
    assert(from_view == 321);

    for (auto& value : empty | hsd::views::reverse)
    {
        (void)value;
        assert(false);
    }

    const hsd::string_view text = "dir/file"_sv;
    assert(*text.rbegin() == 'e');
    hsd::usize steps = 0;
    for (auto it = text.rbegin(); it != text.rend(); ++it)
    {
        ++steps;
    }
    assert(steps == text.length());
    const hsd::string_view blank{};
    assert(blank.rbegin() == blank.rend());

    hsd::static_vector<hsd::i32, 3> fixed;
    fixed.emplace_back(4);
    fixed.emplace_back(5);
    fixed.emplace_back(6);
    const auto& read_only = fixed;
    const hsd::i32* first = read_only.data();
    assert(first[0] == 4 && first[2] == 6);
}
