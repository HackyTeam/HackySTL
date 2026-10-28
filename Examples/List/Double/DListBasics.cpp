// Title: Doubly linked list basics
// Level: Basic
// Shows: push and pop at both ends, front/back, iteration both ways, and erase.

#include <Containers/List.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename List>
void print_forward(const char* label, List& values)
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
    hsd::list<hsd::i32> values;

    // A list grows at either end in constant time.
    values.push_back(2);
    values.push_back(3);
    values.push_front(1);
    values.emplace_back(4);
    print_forward("forward", values);
    hsd::println("size {} front {} back {}"_fmt, values.size(), values.front(), values.back());

    // Nodes are linked both ways, so the list can also be walked backwards.
    // rbegin() is the last node and stepping with -- moves towards the head.
    hsd::print("backward:"_fmt);
    for (auto iter = values.rbegin(); iter != values.end(); --iter)
    {
        hsd::print(" {}"_fmt, *iter);
    }
    hsd::println(""_fmt);

    values.pop_front();
    values.pop_back();
    print_forward("after pops", values);

    // erase takes an iterator and returns a result holding the next position.
    values.push_back(9);
    auto second = values.begin();
    ++second;
    auto next = values.erase(second);
    if (next.is_ok())
    {
        hsd::println("erased, next value is {}"_fmt, *next.unwrap());
    }
    print_forward("after erase", values);

    // Erasing the end iterator is refused instead of crashing.
    auto refused = values.erase(values.end());
    hsd::println("erase(end) ok: {}"_fmt, refused.is_ok() ? "true" : "false");

    values.clear();
    hsd::println("empty after clear: {}"_fmt, values.empty() ? "true" : "false");

    return 0;
}
