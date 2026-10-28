#include <Containers/StringView.hpp>
#include <cassert>

static void test_starts_with()
{
    const hsd::string_view text{"abc"};
    const hsd::string_view empty{};

    assert(text.starts_with('a'));
    assert(!text.starts_with('c'));
    assert(!empty.starts_with('a'));

    assert(text.starts_with(hsd::string_view{"ab"}));
    assert(!text.starts_with(hsd::string_view{"abcd"}));
    assert(empty.starts_with(hsd::string_view{}));
}

static void test_ends_with()
{
    const hsd::string_view text{"abc"};
    const hsd::string_view empty{};

    assert(text.ends_with('c'));
    assert(!text.ends_with('a'));
    assert(!empty.ends_with('c'));

    assert(text.ends_with(hsd::string_view{"bc"}));
    assert(text.ends_with(hsd::string_view{"abc"}));
    assert(!text.ends_with(hsd::string_view{"ab"}));
    assert(text.ends_with(hsd::string_view{}));
    assert(empty.ends_with(hsd::string_view{}));

    assert(text.ends_with("bc"));
    assert(!text.ends_with("xbc"));
}

// A needle longer than the view can never match. Needles one character longer
// than the view used to wrap the offset and report a match.
static void test_longer_needle()
{
    const hsd::string_view ab{"ab"};

    assert(!ab.ends_with(hsd::string_view{"abc"}));
    assert(!ab.ends_with("xab"));
    assert(!ab.ends_with("abcd"));
}

// A zero-length view over longer text must not read the characters past its end.
static void test_zero_length_view()
{
    const hsd::string_view hidden{"abc", 0};

    assert(!hidden.starts_with('a'));
    assert(!hidden.ends_with('c'));
}

int main()
{
    test_starts_with();
    test_ends_with();
    test_longer_needle();
    test_zero_length_view();
    return 0;
}
