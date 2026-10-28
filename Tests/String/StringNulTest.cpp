#include <Containers/String.hpp>
#include <cassert>
#include <random>
#include <string>
#include <vector>

// Embedded nulls are ordinary characters for construction, copy, concatenation,
// ==, <=>, find and rfind, against strings and views.

static std::string to_std(const hsd::string& text)
{
    return std::string(text.c_str(), text.length());
}

// Symbols include the null character and bytes above 0x7f.
static char random_symbol(std::mt19937_64& rng)
{
    static const unsigned char symbols[] = {0x00, 0x01, 0x7f, 0x80, 0xff};
    return static_cast<char>(symbols[rng() % 5]);
}

static void test_embedded_nulls()
{
    const hsd::string_view left_text{"a\0b", 3};
    const hsd::string_view right_text{"a\0c", 3};
    const hsd::string lhs{left_text};
    const hsd::string rhs{right_text};

    assert(!(lhs == rhs));
    assert((lhs <=> rhs) < 0);
    assert((rhs <=> lhs) > 0);

    assert(lhs == hsd::string{left_text});
    assert(!(lhs == right_text));
    assert((lhs <=> right_text) < 0);
}

// A proper prefix sorts first, as in std::string.
static void test_prefix_order()
{
    const hsd::string short_text{"ab"};
    const hsd::string long_text{"abc"};

    assert((short_text <=> long_text) < 0);
    assert((long_text <=> short_text) > 0);
    assert(!(short_text == long_text));
}

static void test_copy()
{
    const hsd::string original{hsd::string_view{"a\0b", 3}};

    const hsd::string copied = original;
    assert(copied.length() == 3 && copied == original);

    hsd::string assigned{"zz"};
    assigned = original;
    assert(assigned.length() == 3 && assigned[2] == 'b');
}

static void test_concatenation()
{
    const hsd::string_view left_text{"a\0b", 3};
    const hsd::string_view right_text{"x\0y", 3};

    hsd::string text{left_text};
    text += right_text;
    assert(text.length() == 6);
    assert(text[2] == 'b' && text[3] == 'x' && text[4] == '\0' && text[5] == 'y');

    const hsd::string joined = text + hsd::string{hsd::string_view{"\0z", 2}};
    assert(joined.length() == 8 && joined[7] == 'z');

    // A view on the left: this used to produce an empty string.
    const hsd::string from_view = left_text + hsd::string{"r"};
    assert(from_view.length() == 4 && from_view[2] == 'b' && from_view[3] == 'r');

    const hsd::string from_cstr = "p" + hsd::string{hsd::string_view{"\0q", 2}};
    assert(from_cstr.length() == 3 && from_cstr[2] == 'q');
}

static void test_find_with_embedded_nulls()
{
    // a(0) \0(1) b(2) x(3) \0(4) y(5)
    const hsd::string text{hsd::string_view{"a\0bx\0y", 6}};
    const hsd::string null_b{hsd::string_view{"\0b", 2}};
    const hsd::string null_y{hsd::string_view{"\0y", 2}};
    const hsd::string null_null{hsd::string_view{"\0\0", 2}};
    const hsd::string null_only{hsd::string_view{"\0", 1}};

    assert(text.find(null_b) == 1);
    assert(text.find(null_y) == 4);
    assert(text.find(null_null) == hsd::string::npos);
    assert(text.rfind(null_only) == 4);
}

// Random short strings compared with std::string: equality, ordering, both
// concatenations, and find and rfind of a needle with embedded nulls.
static void test_random_against_std()
{
    std::mt19937_64 rng(777);
    std::vector<char> lhs;
    std::vector<char> rhs;
    std::vector<char> needle;

    for (int round = 0; round < 3000; ++round)
    {
        lhs.assign(rng() % 8, '\0');
        rhs.assign(rng() % 8, '\0');
        needle.assign(rng() % 3, '\0');

        for (auto& symbol : lhs)
        {
            symbol = random_symbol(rng);
        }

        for (auto& symbol : rhs)
        {
            symbol = random_symbol(rng);
        }

        for (auto& symbol : needle)
        {
            symbol = random_symbol(rng);
        }

        const hsd::string actual_lhs{hsd::string_view{lhs.data(), lhs.size()}};
        const hsd::string actual_rhs{hsd::string_view{rhs.data(), rhs.size()}};
        const hsd::string actual_needle{hsd::string_view{needle.data(), needle.size()}};
        const std::string expected_lhs(lhs.begin(), lhs.end());
        const std::string expected_rhs(rhs.begin(), rhs.end());
        const std::string expected_needle(needle.begin(), needle.end());

        assert((actual_lhs == actual_rhs) == (expected_lhs == expected_rhs));

        const int expected_order = expected_lhs.compare(expected_rhs);
        const int actual_order = actual_lhs <=> actual_rhs;
        assert((actual_order < 0) == (expected_order < 0));
        assert((actual_order > 0) == (expected_order > 0));

        assert(to_std(actual_lhs + actual_rhs) == expected_lhs + expected_rhs);

        hsd::string appended = actual_lhs;
        appended += actual_rhs;
        assert(to_std(appended) == expected_lhs + expected_rhs);

        assert(actual_lhs.find(actual_needle) == expected_lhs.find(expected_needle));
        assert(actual_lhs.rfind(actual_needle) == expected_lhs.rfind(expected_needle));
    }
}

int main()
{
    test_embedded_nulls();
    test_prefix_order();
    test_copy();
    test_concatenation();
    test_find_with_embedded_nulls();
    test_random_against_std();
    return 0;
}
