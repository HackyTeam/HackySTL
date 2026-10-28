#include <Containers/String.hpp>
#include <cassert>

// `hsd::basic_string` reports its storage size including the terminator, so the text
// is compared through `length()`.
template <typename CharT>
static bool same_text(const hsd::basic_string<CharT>& text, const CharT* expected)
{
    hsd::usize _idx = 0;

    for (; expected[_idx] != static_cast<CharT>(0); ++_idx)
    {
        if (_idx >= text.length() || text.c_str()[_idx] != expected[_idx])
        {
            return false;
        }
    }

    return _idx == text.length();
}

static void check_narrow_conversions()
{
    assert(same_text(hsd::to_string(42), "42"));
    assert(same_text(hsd::to_string(-7), "-7"));
    assert(same_text(hsd::to_string(0), "0"));
    assert(same_text(hsd::to_string(static_cast<hsd::u8>(255)), "255"));
    assert(same_text(hsd::to_string(2.5), "2.500000"));
    assert(same_text(hsd::to_string(-1.5), "-1.500000"));
}

static void check_wide_conversions()
{
    assert(same_text(hsd::to_wstring(42), L"42"));
    assert(same_text(hsd::to_wstring(-1.5), L"-1.500000"));
    assert(same_text(hsd::to_u8string(42), u8"42"));
    assert(same_text(hsd::to_u16string(42), u"42"));
    assert(same_text(hsd::to_u32string(42), U"42"));
}

// The lvalue overloads return the string they are given, without copying it.
static void check_string_passthrough()
{
    hsd::string _text = "abc";
    hsd::string& _same = hsd::to_string(_text);

    assert(&_same == &_text);
    assert(same_text(_same, "abc"));
}

int main()
{
    check_narrow_conversions();
    check_wide_conversions();
    check_string_passthrough();

    return 0;
}
