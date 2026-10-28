#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Support/TestSupport.hpp>

static_assert(hsd::string_view{"abc"}.find(hsd::string_view{"bc"}) == 1);
static_assert(hsd::string_view{"abc"}.find(hsd::string_view{""}, 3) == 3);

static void compare_string(const hsd::string& actual, const std::string& expected)
{
    test_support::expect_string(actual, expected);
    assert(actual.size() == expected.size() + 1);
}

static void test_string_against_std()
{
    hsd::string actual;
    std::string expected;

    for (int step = 0; step < 500; ++step)
    {
        switch (step % 10)
        {
            case 0:
            case 1:
            case 2:
            {
                const char value = static_cast<char>('a' + step % 26);
                actual.push_back(value);
                expected.push_back(value);
                break;
            }
            case 3:
                if (!expected.empty())
                {
                    actual.pop_back();
                    expected.pop_back();
                }
                break;
            case 4:
                actual.resize(static_cast<std::size_t>(step % 41) + 1);
                expected.resize(static_cast<std::size_t>(step % 41));
                break;
            case 5:
                actual.reserve(static_cast<std::size_t>(step + 1));
                expected.reserve(static_cast<std::size_t>(step + 1));
                break;
            case 6:
                if (!expected.empty())
                {
                    const auto index = static_cast<std::size_t>(step) % expected.size();
                    actual.erase(actual.begin() + index).unwrap();
                    expected.erase(expected.begin() + index, expected.begin() + index + 1);
                }
                break;
            case 7:
                if (expected.size() > 3)
                {
                    const auto index = static_cast<std::size_t>(step) % expected.size();
                    const auto length = static_cast<std::size_t>(step % 4);
                    const auto end = std::min(index + length, expected.size());
                    actual.erase_for(actual.begin() + index, actual.begin() + end).unwrap();
                    expected.erase(index, end - index);
                }
                break;
            case 8:
                if (expected.find('\0') == std::string::npos)
                {
                    assert(actual.find('a') == expected.find('a'));
                    assert(actual.rfind('z') == expected.rfind('z'));
                }
                break;
            case 9:
                if (expected.size() >= 3)
                {
                    if (expected.find('\0') == std::string::npos)
                    {
                        assert(actual.starts_with(expected.substr(0, 2).c_str()));
                        const auto suffix = expected.substr(expected.size() - 2);
                        assert(actual.ends_with(suffix.c_str()));
                    }
                }
                break;
        }

        if (actual.length() != expected.size())
        {
            std::println(
                stderr,
                "string mismatch after step {}: actual={} expected={}",
                step, actual.length(), expected.size()
            );
        }
        compare_string(actual, expected);
    }
}

static void test_string_search_and_substrings()
{
    hsd::string actual{"abracadabra"};
    compare_string(actual, "abracadabra");

    assert(actual.find('c') == 4);
    assert(actual.find("cada") == 4);
    assert(actual.find('x') == hsd::string::npos);
    assert(actual.rfind('a') == 10);
    assert(actual.rfind("abra") == 7);
    assert(actual.contains("cad"));
    assert(actual.starts_with("abra"));
    assert(actual.ends_with("abra"));

    auto middle = actual.sub_string(3, 4);
    assert(middle.is_ok());
    auto middle_value = middle.unwrap();
    test_support::expect_string(middle_value, "acad");

    auto suffix = actual.sub_string(7);
    assert(suffix.is_ok());
    auto suffix_value = suffix.unwrap();
    test_support::expect_string(suffix_value, "abra");

    assert(!actual.sub_string(actual.length() + 1, 1).is_ok());
}

static void test_string_view_search_against_std()
{
    const std::string text{"abc---abc---ab"};
    const std::string_view expected{text};
    const hsd::string_view actual{text.data(), text.size()};
    const std::string needles[]{"", "abc", "ab", "---", "abcd"};

    for (const auto& needle : needles)
    {
        const std::string_view expected_needle{needle};
        const hsd::string_view actual_needle{needle.data(), needle.size()};

        assert(actual.find(actual_needle) == expected.find(expected_needle));
        for (std::size_t pos = 0; pos <= text.size() + 1; ++pos)
        {
            assert(
                actual.find(actual_needle, pos)
                == expected.find(expected_needle, pos)
            );
        }
    }

    const hsd::string_view empty{};
    const hsd::string_view empty_needle{};
    assert(empty.find(empty_needle) == 0);
    assert(empty.find(empty_needle, 1) == hsd::string_view::npos);
}

static void test_string_special_members()
{
    hsd::string source{"move me"};
    hsd::string copy{source};
    compare_string(copy, "move me");

    source[0] = 'M';
    compare_string(copy, "move me");

    hsd::string moved{hsd::move(source)};
    compare_string(moved, "Move me");

    hsd::string assigned;
    assigned = moved;
    compare_string(assigned, "Move me");
    assigned.clear();
    assert(assigned.length() == 0);
}

static void test_string_edge_cases()
{
    hsd::string value;
    assert(value.length() == 0);
    value.pop_back();
    assert(value.length() == 0);

    value = "abc";
    assert(value.front() == 'a' && value.back() == 'c');
    value.pop_back();
    assert(test_support::to_string(value) == "ab");
    value.erase(value.begin()).unwrap();
    assert(test_support::to_string(value) == "b");
    value.clear();
    assert(value.length() == 0);

    hsd::string resized;
    resized.resize(6);
    assert(resized.length() == 5);
    resized.resize(3);
    assert(resized.length() == 2);
}

int main()
{
    test_string_against_std();
    test_string_search_and_substrings();
    test_string_view_search_against_std();
    test_string_special_members();
    test_string_edge_cases();
}
