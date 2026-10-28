// Title: Heterogeneous bag
// Level: Intermediate
// Shows: a vector of any holding mixed types, dispatching on the stored type, and user-defined payloads.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Any.hpp>

using namespace hsd::format_literals;

struct point
{
    hsd::i32 x;
    hsd::i32 y;
};

// Tries each type the bag is expected to hold and prints a description.
// holds_type compares the exact stored type, so "text" below must be a hsd::string.
static void describe(const hsd::any& item)
{
    if (const auto* number = item.cast_if<hsd::i32>())
    {
        hsd::println("int {}"_fmt, *number);
    }
    else if (const auto* real = item.cast_if<hsd::f64>())
    {
        hsd::println("double {prec=2}"_fmt, *real);
    }
    else if (const auto* text = item.cast_if<hsd::string>())
    {
        hsd::println("string '{}' (length {})"_fmt, text->c_str(), text->length());
    }
    else if (const auto* where = item.cast_if<point>())
    {
        hsd::println("point ({}, {})"_fmt, where->x, where->y);
    }
    else if (!item.has_value())
    {
        hsd::println("empty"_fmt);
    }
    else
    {
        hsd::println("unknown type"_fmt);
    }
}

int main()
{
    hsd::vector<hsd::any> bag;
    bag.push_back(hsd::any{10});
    bag.push_back(hsd::any{3.14159});
    bag.push_back(hsd::any{hsd::string{"hello"}});
    bag.push_back(hsd::any{point{3, -4}});
    bag.push_back(hsd::any{});
    bag.push_back(hsd::any{'c'}); // a char is not one of the handled types

    for (hsd::usize index = 0; index < bag.size(); ++index)
    {
        hsd::print("[{}] "_fmt, index);
        describe(bag[index]);
    }

    // Sum only the ints: other entries simply fail the cast and are skipped.
    hsd::i32 total = 0;
    for (hsd::usize index = 0; index < bag.size(); ++index)
    {
        if (auto* number = bag[index].cast_if<hsd::i32>())
        {
            total += *number;
        }
    }
    hsd::println("int total: {}"_fmt, total);

    // cast_to gives mutable access too: move the point in place.
    auto moved = bag[3].cast_to<point>();
    if (moved.is_ok())
    {
        moved.unwrap().get().x += 10;
    }
    describe(bag[3]);
    return 0;
}
