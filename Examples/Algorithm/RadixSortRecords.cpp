// Title: Radix sort of records
// Level: Intermediate
// Shows: radix_sort on records by an unsigned key, stability, and the default unsigned sort.

#include <Base/Algorithm.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

struct ticket
{
    hsd::u32 priority;
    const char* title;
};

int main()
{
    // Support tickets in the order they arrived. Several share a priority.
    hsd::vector<ticket> tickets{{
        {30, "printer jam"}, {10, "server down"}, {20, "password reset"},
        {10, "disk full"}, {30, "new monitor"}, {20, "vpn timeout"}
    }};

    // The predicate only locates the largest key, which decides how many
    // decimal passes run. The result is always ascending by key, and tickets
    // with equal keys keep their arrival order because each pass is stable.
    hsd::radix_sort(
        tickets,
        [](const ticket& lhs, const ticket& rhs) { return lhs.priority > rhs.priority; },
        [](const ticket& item) { return item.priority; });

    for (const auto& item : tickets)
    {
        hsd::println("{} {}"_fmt, item.priority, item.title);
    }

    // Plain unsigned values need no getter.
    hsd::vector<hsd::u32> ids{{4021, 7, 950, 12, 100000, 88}};
    hsd::radix_sort(ids);

    hsd::print("ids:"_fmt);
    for (const auto& id : ids)
    {
        hsd::print(" {}"_fmt, id);
    }
    hsd::println(""_fmt);

    return 0;
}
