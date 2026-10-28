// Title: Tuple basics
// Level: Basic
// Shows: building a tuple, reading elements by index, make_tuple, concatenation and tie.

#include <Serialize/Io.hpp>
#include <Wrappers/Tuple.hpp>

using namespace hsd::format_literals;

int main()
{
    // Class template argument deduction picks the element types.
    hsd::tuple record = {7, 'x', 2.5};

    // Elements are read with get<Index>(); there are no structured bindings for tuples.
    hsd::println("record: {} {} {prec=1}"_fmt,
        record.get<0>(), record.get<1>(), record.get<2>());

    // get<Index>() returns a reference, so it can also assign.
    record.get<0>() = 8;
    hsd::println("changed first: {}"_fmt, record.get<0>());

    // make_tuple decays its arguments, like std::make_tuple.
    auto made = hsd::make_tuple(-1, 100u);
    hsd::println("made: {} {}"_fmt, made.get<0>(), made.get<1>());

    // operator+ joins two tuples into a longer one.
    auto joined = record + made;
    hsd::println("joined: {} {} {prec=1} {} {}"_fmt,
        joined.get<0>(), joined.get<1>(), joined.get<2>(), joined.get<3>(), joined.get<4>());

    // tie builds a tuple of references to existing variables.
    hsd::i32 target = 1;
    auto refs = hsd::tie(target);
    refs.get<0>() = 10;
    hsd::println("variable after tie: {}"_fmt, target);

    // Copies are independent of the original.
    auto copy = record;
    copy.get<0>() = 99;
    hsd::println("original {} copy {}"_fmt, record.get<0>(), copy.get<0>());
    // tie() refers to existing variables, so assigning through it changes them.
    hsd::i32 low = 1;
    hsd::i32 high = 2;
    auto bounds = hsd::tie(low, high);
    bounds.get<0>() = 10;
    bounds.get<1>() = 20;
    hsd::println("tied: {} {}"_fmt, low, high);

    // make_tuple copies its arguments, including a const lvalue tuple.
    const hsd::tuple<hsd::i32> source{7};
    auto copied = hsd::make_tuple(source, hsd::i32{3});
    hsd::println("copied: {} {}"_fmt, copied.get<0>().get<0>(), copied.get<1>());
    return 0;
}
