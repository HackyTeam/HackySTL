#include <cassert>

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Range.hpp>
#include <Containers/StackArray.hpp>
#include <Support/TestSupport.hpp>

static hsd::string trim(hsd::string_view input)
{
    hsd::string result;
    auto view = input
        | hsd::views::drop_while(hsd::cstring::iswhitespace)
        | hsd::views::reverse
        | hsd::views::drop_while(hsd::cstring::iswhitespace);

    for (auto value : view | hsd::views::reverse)
    {
        result.push_back(value);
    }
    return result;
}

int main()
{
    test_support::expect_string(
        trim(" \f\n\t\r\vHello, C++20!\f\n\t\r\v "),
        "Hello, C++20!"
    );
    test_support::expect_string(trim("   "), "");
    test_support::expect_string(trim("value"), "value");

    constexpr hsd::stack_array values = {0, 1, 2, 3, 4, 5};
    int first = -1;
    for (auto value : values | hsd::views::drop_while([](int value)
    {
        return value < 3;
    }))
    {
        if (first == -1)
        {
            first = value;
        }
        assert(value >= 3);
    }
    assert(first == 3);
}
