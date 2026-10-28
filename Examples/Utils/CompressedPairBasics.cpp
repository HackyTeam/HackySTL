// Title: Compressed pair
// Level: Basic
// Shows: compressed_pair accessors, indexed get, and the space saved for an empty member.

#include <Extra/CompressedPair.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Pair.hpp>

using namespace hsd::format_literals;

// A type with no data, like a comparator or a stateless allocator.
struct empty_policy {};

const char* yes_no(bool value)
{
    return value ? "true" : "false";
}

int main()
{
    // Two ordinary members cost the same as a regular pair.
    hsd::compressed_pair<int, float> values(3, 4.5f);
    hsd::println("first={} second={prec=1}"_fmt, values.first(), values.second());

    // first() and second() return references, so they can assign.
    values.first() = 9;
    values.second() = 2.5f;
    hsd::println("get<0>={} get<1>={prec=1}"_fmt, values.get<0>(), values.get<1>());

    // An empty member takes no room of its own, unlike a member of a pair.
    hsd::compressed_pair<int, empty_policy> counter(7, empty_policy{});
    hsd::compressed_pair<empty_policy, int> flipped(empty_policy{}, 8);

    hsd::println("same size as pair: {}"_fmt,
        yes_no(sizeof(values) == sizeof(hsd::pair<int, float>)));
    hsd::println("int with empty is just an int: {}"_fmt,
        yes_no(sizeof(counter) == sizeof(int)));
    hsd::println("empty first is also free: {}"_fmt,
        yes_no(sizeof(flipped) == sizeof(int)));
    hsd::println("pair pays for the empty one: {}"_fmt,
        yes_no(sizeof(hsd::pair<int, empty_policy>) > sizeof(int)));

    hsd::println("kept values: {} and {}"_fmt, counter.first(), flipped.second());

    return 0;
}
