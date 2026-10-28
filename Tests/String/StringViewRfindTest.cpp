#include <Containers/StringView.hpp>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string_view>
#include <vector>

static void test_empty_needle()
{
    const hsd::string_view text{"abc"};
    const hsd::string_view empty{};

    assert(text.rfind(empty) == 3);
    assert(text.rfind(empty, 1) == 1);
    assert(hsd::string_view{}.rfind(empty) == 0);
}

static void test_repeated_needles()
{
    const hsd::string_view repeated{"abcabc"};
    const hsd::string_view a{"a"};
    const hsd::string_view bc{"bc"};

    assert(repeated.rfind(a, 2) == 0);
    assert(repeated.rfind(a) == 3);
    assert(repeated.rfind(bc, 4) == 4);
    assert(repeated.rfind(bc) == 4);
    assert(repeated.rfind(hsd::string_view{"abcabc"}) == 0);
    assert(repeated.rfind(hsd::string_view{"abcabcd"}) == hsd::string_view::npos);
    assert(repeated.rfind(hsd::string_view{"cab"}, 1) == hsd::string_view::npos);
}

static void test_no_read_past_end()
{
    // The view covers exactly two heap bytes, so no needle may read past them.
    char* buffer = static_cast<char*>(std::malloc(2));
    std::memcpy(buffer, "ab", 2);
    const hsd::string_view tight{buffer, 2};

    assert(tight.rfind(hsd::string_view{"bb"}) == hsd::string_view::npos);
    assert(tight.rfind(hsd::string_view{"ab"}) == 0);

    std::free(buffer);
}

// A start position past the end is clamped, so a match may begin at the end of
// the view, as in std::string_view. A character before `pos` never matches later.
static void test_position_bounds()
{
    const hsd::string_view text{"abc"};

    assert(text.rfind('c', 3) == 2);
    assert(text.rfind('c', 9) == 2);
    assert(text.rfind('c', 1) == hsd::string_view::npos);
    assert(text.rfind('a', 1) == 0);
    assert(hsd::string_view{}.rfind('a', 0) == hsd::string_view::npos);

    assert(text.rfind(hsd::string_view{"c"}, 3) == 2);
    assert(text.rfind(hsd::string_view{"bc"}, 2) == 1);
    assert(text.rfind(hsd::string_view{"bc"}, 0) == hsd::string_view::npos);
    assert(text.rfind(hsd::string_view{}, 3) == 3);
    assert(text.rfind(hsd::string_view{}, 1) == 1);
}

// Random short views over three letters, with positions past the end, compared
// with std::string_view.
static void test_random_against_std()
{
    std::mt19937_64 rng(31);
    std::vector<char> haystack;
    std::vector<char> needle;

    for (int round = 0; round < 3000; ++round)
    {
        haystack.assign(rng() % 9, 'a');
        needle.assign(rng() % 3, 'a');

        for (auto& letter : haystack)
        {
            letter = static_cast<char>('a' + rng() % 3);
        }

        for (auto& letter : needle)
        {
            letter = static_cast<char>('a' + rng() % 3);
        }

        const hsd::string_view actual_text{haystack.data(), haystack.size()};
        const std::string_view expected_text{haystack.data(), haystack.size()};
        const hsd::string_view actual_needle{needle.data(), needle.size()};
        const std::string_view expected_needle{needle.data(), needle.size()};

        for (std::size_t pos = 0; pos <= 10; ++pos)
        {
            const char letter = static_cast<char>('a' + pos % 3);
            assert(actual_text.rfind(letter, pos) == expected_text.rfind(letter, pos));
            assert(actual_text.rfind(actual_needle, pos) == expected_text.rfind(expected_needle, pos));
        }

        assert(actual_text.rfind(actual_needle) == expected_text.rfind(expected_needle));
    }
}

int main()
{
    test_empty_needle();
    test_repeated_needles();
    test_no_read_past_end();
    test_position_bounds();
    test_random_against_std();
    return 0;
}
