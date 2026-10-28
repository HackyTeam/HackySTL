// Title: Heap array basics
// Level: Basic
// Shows: heap_array construction from a C array, checked at(), and iteration.

#include <Containers/HeapArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    // The element type and the count are deduced from the C array. The
    // elements live in one allocation made by the default allocator.
    hsd::heap_array values{{1, 2, 3, 4, 5}};
    hsd::println("size: {}"_fmt, values.size());

    hsd::print("values:"_fmt);
    for (const auto& value : values)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);

    values[2] = 30;
    hsd::println("values[2]={}"_fmt, values[2]);

    // at() returns a result, so a bad index is an error and not a crash.
    auto inside = values.at(4);
    hsd::println("at(4) ok={} value={}"_fmt,
        inside.is_ok() ? "true" : "false", inside.unwrap().get());

    auto outside = values.at(5);
    hsd::println("at(5) ok={}"_fmt, outside.is_ok() ? "true" : "false");
    if (!outside.is_ok())
    {
        hsd::println("error: {}"_fmt, outside.unwrap_err().pretty_error());
    }

    // The default constructor value-initializes every element.
    hsd::heap_array<hsd::i32, 3> zeros;
    hsd::println("zeros: {} {} {}"_fmt, zeros[0], zeros[1], zeros[2]);

    return 0;
}
