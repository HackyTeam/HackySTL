// Title: Stack trace handles
// Level: Basic
// Shows: stack_trace::add as an RAII call-site marker, and reading the top entry.

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/Logging.hpp>

using namespace hsd::format_literals;

static void inner()
{
    // The entry is pushed here and popped when 'here' goes out of scope.
    hsd::stack_trace here = hsd::stack_trace::add();

    // get() returns the most recent call site, which carries the function name.
    hsd::println("inner sees function {}"_fmt, here.get().function_name());
}

static void outer()
{
    hsd::stack_trace here = hsd::stack_trace::add();
    hsd::println("outer sees function {}"_fmt, here.get().function_name());
    inner();

    // inner's entry was popped when it returned, so ours is on top again.
    hsd::println("back in {}"_fmt, here.get().function_name());
}

int main()
{
    // The line is captured by add(), so it matches the line where we called it.
    const hsd::i32 expected_line = __LINE__ + 1;
    hsd::stack_trace entry = hsd::stack_trace::add();
    hsd::println("line recorded correctly: {}"_fmt,
        entry.get().line() == expected_line ? "true" : "false");

    outer();

    // A default handle owns nothing, so destroying it pops nothing.
    {
        hsd::stack_trace idle;
    }
    hsd::println("main still on top: {}"_fmt, entry.get().function_name());

    return 0;
}
