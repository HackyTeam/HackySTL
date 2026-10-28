#include <Containers/StringView.hpp>
#include <cassert>
#include <random>
#include <string_view>
#include <vector>

constexpr hsd::string_view left{"a\0b", 3};
constexpr hsd::string_view right{"a\0c", 3};
constexpr hsd::string_view haystack{"x\0a\0b", 5};

static_assert(left.find(right) == hsd::string_view::npos);
static_assert(!(left == right));
static_assert((left <=> right) < 0);
static_assert((right <=> left) > 0);
static_assert(left == hsd::string_view{"a\0b", 3});
static_assert(!(hsd::string_view{"a\0", 2} == hsd::string_view{"a", 1}));

// A proper prefix sorts first, so `<` and `<=` agree with `<=>`.
static_assert((hsd::string_view{"ab"} <=> hsd::string_view{"abc"}) < 0);
static_assert((hsd::string_view{"abc"} <=> hsd::string_view{"ab"}) > 0);
static_assert(hsd::string_view{"ab"} < hsd::string_view{"abc"});
static_assert(hsd::string_view{"ab"} <= hsd::string_view{"abc"});
static_assert(!(hsd::string_view{"abc"} <= hsd::string_view{"ab"}));

// Bytes above 0x7f sort after ASCII, as with strcmp and std::string_view.
static_assert((hsd::string_view{"\x80"} <=> hsd::string_view{"a"}) > 0);
static_assert(hsd::cstring::compare("\x80", "a") > 0);

static_assert(haystack.find(hsd::string_view{"\0b", 2}) == 3);
static_assert(haystack.find(hsd::string_view{"\0\0", 2}) == hsd::string_view::npos);
static_assert(haystack.rfind(hsd::string_view{"\0", 1}) == 3);

// Symbols include the null character and bytes above 0x7f.
constexpr unsigned symbols[] = {0x00, 0x01, 0x7f, 0x80, 0xff};

template <typename CharT, typename Rng>
static void test_random_against_std(Rng& rng, int rounds)
{
    std::vector<CharT> lhs;
    std::vector<CharT> rhs;

    for (int round = 0; round < rounds; ++round)
    {
        lhs.assign(rng() % 8, CharT{});
        rhs.assign(rng() % 8, CharT{});

        for (auto& symbol : lhs)
        {
            symbol = static_cast<CharT>(symbols[rng() % 5]);
        }

        for (auto& symbol : rhs)
        {
            symbol = static_cast<CharT>(symbols[rng() % 5]);
        }

        const hsd::basic_string_view<CharT> actual_lhs{lhs.data(), lhs.size()};
        const hsd::basic_string_view<CharT> actual_rhs{rhs.data(), rhs.size()};
        const std::basic_string_view<CharT> expected_lhs{lhs.data(), lhs.size()};
        const std::basic_string_view<CharT> expected_rhs{rhs.data(), rhs.size()};

        assert((actual_lhs == actual_rhs) == (expected_lhs == expected_rhs));
        assert(actual_lhs.find(actual_rhs) == expected_lhs.find(expected_rhs));
        assert(actual_lhs.rfind(actual_rhs) == expected_lhs.rfind(expected_rhs));

        // <=> orders whole views, as std::string_view does.
        const int expected = expected_lhs.compare(expected_rhs);
        const int actual = actual_lhs <=> actual_rhs;

        assert((actual < 0) == (expected < 0));
        assert((actual > 0) == (expected > 0));
    }
}

static void test_embedded_nulls()
{
    // A null at the end of a view is a character, so it is found and it makes
    // the view longer than the same characters without it.
    const hsd::string_view wide_null{"ab\0", 3};
    assert(wide_null.find(hsd::string_view{"b\0", 2}) == 1);
    assert(wide_null.rfind(hsd::string_view{"\0", 1}) == 2);
    assert(!(wide_null == hsd::string_view{"ab", 2}));

    // Run-time values take the memcmp path rather than the constant-evaluated one.
    const hsd::string_view runtime_lhs{"a\0b", 3};
    const hsd::string_view runtime_rhs{"a\0c", 3};
    assert(runtime_lhs.find(runtime_rhs) == hsd::string_view::npos);
    assert(!(runtime_lhs == runtime_rhs));
    assert((runtime_lhs <=> runtime_rhs) < 0);
    assert((runtime_rhs <=> runtime_lhs) > 0);
}

int main()
{
    std::mt19937_64 rng(4242);

    test_random_against_std<char>(rng, 4000);
    test_random_against_std<unsigned char>(rng, 4000);
    test_random_against_std<wchar_t>(rng, 4000);
    test_random_against_std<char16_t>(rng, 4000);
    test_random_against_std<char32_t>(rng, 4000);

    test_embedded_nulls();
    return 0;
}
