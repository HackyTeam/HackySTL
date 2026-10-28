// Title: Span basics
// Level: Basic
// Shows: viewing a container with span, size, take, drop, enumerate, and editing through a view.

#include <Containers/Span.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename View>
void print_all(const char* label, const View& view)
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
    hsd::vector<hsd::i32> numbers{{10, 20, 30, 40, 50}};

    // A span is a non-owning view: it holds iterators, not copies.
    auto view = numbers.to_span();
    hsd::println("size: {}"_fmt, view.size());
    print_all("view", view);

    // take() keeps the first n elements and drop() skips them. Both return a
    // result, because n can be larger than the view.
    print_all("take 2", view.take(2).unwrap());
    print_all("drop 3", view.drop(3).unwrap());
    print_all("drop 1, take 3", view.drop(1).unwrap().take(3).unwrap());

    auto too_far = view.drop(6);
    hsd::println("drop 6 ok: {}"_fmt, too_far.is_ok() ? "true" : "false");

    // Writing through the view changes the vector.
    for (auto& value : view)
    {
        value /= 10;
    }
    hsd::println("numbers[4]={}"_fmt, numbers[4]);

    // enumerate() yields [value, index] pairs, value first.
    for (auto [value, index] : view.enumerate())
    {
        hsd::println("index {} holds {}"_fmt, index, value);
    }

    return 0;
}
