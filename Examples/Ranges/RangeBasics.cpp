// Title: Range views basics
// Level: Basic
// Shows: drop, take, drop_while, take_while, reverse and span enumerate.

#include <Containers/Range.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Views are lazy: they only describe a window over the vector, so nothing is
// copied until the loop below walks it.
template <typename View>
void print_view(const char* label, View view)
{
    hsd::print("{}:"_fmt, label);
    for (const auto& value : view)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
}

int main()
{
    hsd::vector<hsd::i32> numbers{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};

    // A view is built by piping a container into an adaptor.
    print_view("drop 7", numbers | hsd::views::drop(7));
    print_view("take 3", numbers | hsd::views::take(3));

    // Adaptors chain: skip two, then keep three of what is left.
    print_view("drop 2 take 3", numbers | hsd::views::drop(2) | hsd::views::take(3));

    // The *_while forms stop at the first element that fails the test.
    print_view("take_while < 4", numbers | hsd::views::take_while([](hsd::i32 value)
    {
        return value < 4;
    }));
    print_view("drop_while < 8", numbers | hsd::views::drop_while([](hsd::i32 value)
    {
        return value < 8;
    }));

    // reverse is an object, so it has no parentheses.
    print_view("reverse", numbers | hsd::views::reverse);
    print_view("last three", numbers | hsd::views::reverse | hsd::views::take(3));

    // A view reports how many elements it covers.
    auto tail = numbers | hsd::views::drop(7);
    hsd::println("tail has {} elements"_fmt, tail.size());

    // enumerate pairs each element with its index, value first.
    hsd::print("indexed:"_fmt);
    for (auto [value, index] : numbers.to_span().enumerate())
    {
        if (index < 3)
        {
            hsd::print(" {}@{}"_fmt, value, index);
        }
    }
    hsd::println(""_fmt);

    return 0;
}
