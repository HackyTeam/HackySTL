// Title: Stack array basics
// Level: Basic
// Shows: stack_array construction, checked at(), fill, swap and iteration.

#include <Containers/StackArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename Arr>
void print_all(const char* label, const Arr& values)
{
    hsd::print("{}:"_fmt, label);
    for (const auto& value : values)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
}

int main()
{
    // The element type and the count are deduced from the initializer list.
    hsd::stack_array values{10, 20, 30, 40, 50};
    print_all("values", values);
    hsd::println("size: {}"_fmt, values.size());

    // operator[] is unchecked, at() returns a result instead.
    values[1] = 25;
    hsd::println("values[1]={}"_fmt, values[1]);

    auto inside = values.at(4);
    hsd::println("at(4) ok={} value={}"_fmt,
        inside.is_ok() ? "true" : "false", inside.unwrap().get());

    auto outside = values.at(5);
    hsd::println("at(5) ok={}"_fmt, outside.is_ok() ? "true" : "false");

    // fill() copies one value into every slot.
    hsd::stack_array<hsd::i32, 5> other;
    other.fill(7);
    print_all("other", other);

    // swap() exchanges two arrays of the same type and size element by element.
    hsd::swap(values, other);
    print_all("values", values);
    print_all("other", other);

    return 0;
}
