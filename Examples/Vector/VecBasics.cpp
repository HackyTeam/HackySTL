// Title: Vector basics
// Level: Basic
// Shows: push_back, emplace_back, checked at(), erase, resize and reserve on hsd::vector.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

void print_all(const char* label, const hsd::vector<hsd::i32>& values)
{
    hsd::print("{} (size {}):"_fmt, label, values.size());
    for (const auto& value : values)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
}

int main()
{
    // A braced list needs a second pair of braces: the first builds the array
    // that the constructor copies from.
    hsd::vector<hsd::i32> values{{10, 20, 30}};
    print_all("start", values);

    values.push_back(40);
    values.emplace_back(50);
    print_all("grown", values);
    hsd::println("ends: {} and {}"_fmt, values.front(), values.back());

    // at() checks the index and returns a result; operator[] does not check.
    auto second = values.at(1);
    if (second.is_ok())
    {
        hsd::println("at(1)={}"_fmt, second.unwrap().get());
    }

    auto missing = values.at(99);
    hsd::println("at(99) is {}"_fmt, missing.is_ok() ? "ok" : "an error");

    values.pop_back();
    values.erase(values.begin() + 1).unwrap();
    print_all("after pop_back and erase", values);

    // resize() value-initializes new elements, so they start as 0.
    values.resize(6);
    print_all("resized up", values);
    values.resize(2);
    print_all("resized down", values);

    // reserve() only changes the capacity, never the size.
    values.reserve(100);
    hsd::println("capacity >= 100: {}"_fmt, values.capacity() >= 100 ? "true" : "false");
    hsd::println("size still {}"_fmt, values.size());

    values.clear();
    hsd::println("cleared, size {}"_fmt, values.size());

    // Reverse traversal of an empty vector visits nothing.
    hsd::vector<hsd::i32> edge_none;
    hsd::usize edge_steps = 0;
    for (auto it = edge_none.rbegin(); it != edge_none.rend(); ++it)
    {
        ++edge_steps;
    }
    hsd::println("empty reverse steps: {}"_fmt, edge_steps);

    // Reverse traversal of a non-empty vector goes from the back to the front.
    hsd::vector<hsd::i32> edge_values{{1, 2, 3}};
    hsd::print("reversed:"_fmt);
    for (auto it = edge_values.rbegin(); it != edge_values.rend(); ++it)
    {
        hsd::print(" {}"_fmt, *it);
    }
    hsd::println(""_fmt);
    return 0;
}
