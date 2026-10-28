// Title: Singly linked list basics
// Level: Basic
// Shows: forward_list push, pop, front/back, iteration, erase and size.

#include <Containers/ForwardList.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename List>
void print_all(const char* label, const List& values)
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
    hsd::forward_list<hsd::i32> values;

    // The list keeps a pointer to both ends, so adding at either one is cheap.
    values.push_back(20);
    values.push_back(30);
    values.push_front(10);
    values.emplace_back(40);
    print_all("values", values);
    hsd::println("size {} front {} back {}"_fmt, values.size(), values.front(), values.back());

    // Nodes link only forward, so there is no pop_back and no backward walk.
    values.pop_front();
    print_all("after pop_front", values);

    // erase takes an iterator and returns a result holding the next position.
    // Walking to a node costs one step per element.
    auto target = values.begin();
    ++target;
    auto next = values.erase(target);
    if (next.is_ok())
    {
        hsd::println("erased 30, next is {}"_fmt, *next.unwrap());
    }
    print_all("after erase", values);

    // Sum with a plain range-for, then empty the list.
    hsd::i32 total = 0;
    for (const auto& value : values)
    {
        total += value;
    }
    hsd::println("sum {}"_fmt, total);

    values.clear();
    hsd::println("size after clear {}"_fmt, values.size());

    // Erasing the last element moves the tail, so the next push_back is safe.
    hsd::forward_list<hsd::i32> edge_list{{1, 2, 3}};
    auto edge_last = edge_list.begin();
    ++edge_last;
    ++edge_last;
    edge_list.erase(edge_last).unwrap();
    edge_list.push_back(9);
    hsd::print("after tail erase:"_fmt);
    for (const auto value : edge_list)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
    return 0;
}
