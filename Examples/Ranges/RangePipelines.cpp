// Title: Range pipelines
// Level: Intermediate
// Shows: chaining filter, transform, drop_while, take and reverse over numbers read from stdin.

#include <Containers/Range.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename View>
void print_view(const char* label, View view)
{
    hsd::print("{}:"_fmt, label);
    hsd::usize shown = 0;
    for (const auto& value : view)
    {
        hsd::print(" {}"_fmt, value);
        ++shown;
    }
    if (shown == 0)
    {
        hsd::print(" (none)"_fmt);
    }
    hsd::println(""_fmt);
}

int main()
{
    hsd::vector<hsd::i32> readings;

    // One reading per line. The stream hands over a value once more input
    // follows it, so the input finishes with a line holding 999.
    for (auto value = hsd::io::cin().read_value<hsd::i32>(); value.is_ok();
        value = hsd::io::cin().read_value<hsd::i32>())
    {
        if (value.unwrap() == 999)
        {
            break;
        }
        readings.push_back(value.unwrap());
    }

    hsd::println("read {} values"_fmt, readings.size());

    // filter keeps the elements the predicate accepts.
    print_view("positive", readings | hsd::ranges::filter([](hsd::i32 value)
    {
        return value > 0;
    }));

    // transform maps each element, but the result keeps the element type.
    // Here: positive values, squared.
    print_view("positive squared", readings
        | hsd::ranges::filter([](hsd::i32 value) { return value > 0; })
        | hsd::ranges::transform([](hsd::i32 value) { return value * value; }));

    // Skip a leading run of non-positive readings, then take up to three.
    // take panics when the range is shorter than asked, so clamp the count
    // to the size of the view first.
    auto live = readings
        | hsd::views::drop_while([](hsd::i32 value) { return value <= 0; });
    const hsd::usize wanted = live.size() < 3 ? live.size() : 3;
    print_view("first three after the dead start", live | hsd::views::take(wanted));

    // Everything up to the first negative value, newest first.
    print_view("before first negative, reversed", readings
        | hsd::views::take_while([](hsd::i32 value) { return value >= 0; })
        | hsd::views::reverse);

    // Views refer to the vector, so writing through a filter edits it in place.
    for (auto& value : readings | hsd::ranges::filter([](hsd::i32 item)
    {
        return item < 0;
    }))
    {
        value = 0;
    }

    hsd::i32 total = 0;
    for (const auto& value : readings)
    {
        total += value;
    }
    hsd::println("after clamping negatives to zero, sum is {}"_fmt, total);

    return 0;
}
