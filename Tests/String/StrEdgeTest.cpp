#include <cassert>

#include <Containers/String.hpp>
#include <Serialize/SStream.hpp>
#include <Support/AbortSupport.hpp>

using namespace hsd::format_literals;

// Edge cases of string search, affix and containment checks, the static string's move,
// resize and erase, and printing into a buffer that is too small for the prefix.
namespace
{
    // Resolves a format text into the single piece it contains, as the printing tests do.
    template <auto Text, hsd::usize Count>
    constexpr auto prefix_format()
    {
        constexpr auto parsed = hsd::parse_literal<Text, Count>().unwrap();
        constexpr auto piece = parsed[0];
        constexpr auto lit = hsd::format_literal<char, piece.length + 1>{
            .format = {piece.format, piece.length},
            .base = piece.base
        };
        return hsd::format<lit>();
    }

    constexpr auto prefix_fmt = prefix_format<("x{}"_fmt)(), 2>();
}

int main()
{
    // rfind searches backward from the position: a match must start at or before it. A match
    // at index 0 is found, and a match after the position is not.
    hsd::string _text{"abcabc"};
    assert(_text.rfind("zz") == hsd::string::npos);
    assert(_text.rfind("bc", 1) == 1);
    assert(_text.rfind("bc") == 4);
    assert(_text.rfind("abc", 2) == 0);
    assert(_text.rfind("abc", 2) == 0 && hsd::string{"abc"}.rfind('a') == 0);
    assert(_text.rfind('z') == hsd::string::npos);
    assert(_text.rfind('c', 2) == 2);
    assert(_text.rfind('c') == 5);
    assert(_text.rfind(hsd::string{"abc"}, 3) == 3);
    assert(_text.rfind(hsd::string{"qq"}) == hsd::string::npos);

    // A position past the end searches the whole string, as the standard does.
    assert(_text.rfind("bc", 10) == 4);
    assert(_text.rfind('c', 10) == 5);
    assert(_text.rfind(hsd::string{"bc"}, 10) == 4);
    assert(_text.rfind(hsd::string{"bc"}, 5) == 4);
    assert(_text.rfind("qq", 10) == hsd::string::npos);

    // Empty strings have no characters to match, and every string ends with the empty one.
    const hsd::string _empty{};
    assert(!_empty.starts_with('a') && !_empty.starts_with("a") && !_empty.starts_with(hsd::string{"a"}));
    assert(!_empty.contains('a') && !_empty.contains("a") && !_empty.contains(hsd::string{"a"}));
    assert(!_empty.ends_with('a'));
    assert(_text.ends_with("") && _text.ends_with(hsd::string{}));

    // Affix and containment checks answer false for missing or longer needles.
    assert(!_text.starts_with('z'));
    assert(!_text.starts_with("abcabcd"));
    assert(!_text.starts_with(hsd::string{"abcabcd"}));
    assert(!_text.ends_with('z'));
    assert(!_text.ends_with("zabcabc"));
    assert(!_text.ends_with(hsd::string{"zabcabc"}));
    assert(!_text.contains('z'));
    assert(!_text.contains("zz"));
    assert(!_text.contains(hsd::string{"zz"}));
    assert(_text.starts_with("abc") && _text.ends_with("abc") && _text.contains('b'));

    // A static string moved into a new one keeps its characters, and the source is emptied.
    hsd::static_string<8> _source{"move"};
    hsd::static_string<8> _moved{hsd::move(_source)};
    assert(_moved.length() == 4);
    assert(_moved == hsd::static_string<8>{"move"});

    // A static string whose characters are removed with a reversed range reports bad access,
    // and a valid range removes the characters between the two positions.
    hsd::static_string<8> _erasable{"erase"};
    assert(!_erasable.erase_for(_erasable.begin() + 3, _erasable.begin() + 1).is_ok());
    assert(_erasable.erase_for(_erasable.begin() + 1, _erasable.begin() + 3).is_ok());
    assert(_erasable == hsd::static_string<8>{"ese"});

    // A basic string refuses a reversed range the same way.
    hsd::string _removable{"abc"};
    assert(!_removable.erase_for(_removable.begin() + 2, _removable.begin()).is_ok());

    // The size given to resize includes the terminator. Shrinking moves the terminator, and
    // growing zero-fills the slots that join the string.
    hsd::static_string<8> _resized{"abcdef"};
    _resized.resize(3);
    assert(_resized.length() == 2 && _resized == hsd::static_string<8>{"ab"});
    _resized.resize(5);
    assert(_resized.length() == 4 && _resized[2] == '\0' && _resized[3] == '\0');

    // Comparing static strings of different lengths is false without looking further.
    assert(!(hsd::static_string<8>{"ab"} == hsd::static_string<8>{"abc"}));

    // Printing with a prefix needs room for it. A buffer with no room is refused.
    char _small[2] = {};
    auto _refused = hsd::print_to<prefix_fmt, char>(_small, 1, hsd::static_string<8>{"v"});
    assert(!_refused.is_ok());

    // Appending past the capacity of a static string ends the process, and so does
    // resizing it beyond its capacity.
    test_support::expect_abort([]
    {
        hsd::static_string<2> _full{"ab"};
        _full.emplace_back('c');
    });
    test_support::expect_abort([]
    {
        hsd::static_string<2> _full{"ab"};
        _full.resize(10);
    });
    test_support::expect_abort([]
    {
        hsd::static_string<2> _full{"ab"};
        _full.push_back('c');
    });
    return 0;
}
