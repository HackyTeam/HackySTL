// Title: Deque basics
// Level: Basic
// Shows: pushing and popping at both ends, indexing, checked at(), erase, and reverse iteration.

#include <Containers/Deque.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename Dq>
void print_all(const char* label, const Dq& values)
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
    hsd::deque<hsd::i32> values;

    // Both ends are cheap: push_back appends, push_front prepends.
    values.push_back(2);
    values.push_back(3);
    values.push_front(1);
    values.emplace_front(0);
    values.emplace_back(4);
    print_all("values", values);
    hsd::println("size: {} front: {} back: {}"_fmt,
        values.size(), values.front(), values.back());

    // operator[] is unchecked, at() returns a result.
    hsd::println("values[2]={}"_fmt, values[2]);
    auto outside = values.at(9);
    hsd::println("at(9) ok: {}"_fmt, outside.is_ok() ? "true" : "false");

    values.pop_front();
    values.pop_back();
    print_all("after pops", values);

    // erase() takes an iterator and returns a result holding the next one.
    auto erased = values.erase(values.begin());
    hsd::println("erase ok: {}"_fmt, erased.is_ok() ? "true" : "false");
    print_all("after erase", values);

    // A deque can also start from a C array and be resized.
    hsd::deque<hsd::i32> from_array{{7, 8, 9}};
    from_array.resize(5);
    print_all("resized", from_array);

    // rbegin() and rend() walk from the last element back to the first.
    hsd::print("reversed:"_fmt);
    for (auto it = from_array.rbegin(); it != from_array.rend(); ++it)
    {
        hsd::print(" {}"_fmt, *it);
    }
    hsd::println(""_fmt);

    // An empty deque has nothing to walk, so its reverse range is empty.
    hsd::deque<hsd::i32> nothing;
    hsd::println("empty reverse is empty: {}"_fmt, nothing.rbegin() == nothing.rend() ? "true" : "false");

    from_array.clear();
    hsd::println("size after clear: {}"_fmt, from_array.size());

    return 0;
}
