// Title: Default arguments
// Level: Basic
// Shows: default_call_t storing fallback values and default_v selecting them at the call.

#include <Extra/DefaultArg.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

hsd::i32 add(hsd::i32 lhs, hsd::i32 rhs)
{
    return lhs + rhs;
}

int main()
{
    // The wrapper stores the function plus one fallback value for each
    // parameter. Here the fallbacks are 3 and 4.
    hsd::default_call_t adder(&add, 3, 4);

    // A real value is passed through as it is.
    hsd::println("add(10, 20) = {}"_fmt, adder(10, 20));

    // default_v at a position asks for the stored fallback.
    hsd::println("add(10, default) = {}"_fmt, adder(10, hsd::default_v));
    hsd::println("add(default, 20) = {}"_fmt, adder(hsd::default_v, 20));
    hsd::println("add(default, default) = {}"_fmt, adder(hsd::default_v, hsd::default_v));

    return 0;
}
