// Title: Error chain with call sites
// Level: Advanced
// Shows: stack_trace_error propagated through nested calls, with stack_trace handles per frame.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/Logging.hpp>

using namespace hsd::format_literals;

using step_result = hsd::option_err<hsd::stack_trace_error>;

// Frames visited on the way down, in call order. Each function also holds a
// stack_trace handle, so the library's own stack mirrors this list.
static hsd::vector<const char*> visited;

static step_result check_range(hsd::i32 value)
{
    hsd::stack_trace here = hsd::stack_trace::add();
    visited.push_back(here.get().function_name());

    if (value < 0 || value > 100)
    {
        return hsd::stack_trace_error{};
    }

    return {};
}

static step_result check_even(hsd::i32 value)
{
    hsd::stack_trace here = hsd::stack_trace::add();
    visited.push_back(here.get().function_name());

    if (value % 2 != 0)
    {
        return hsd::stack_trace_error{};
    }

    return {};
}

static step_result validate(hsd::i32 value)
{
    hsd::stack_trace here = hsd::stack_trace::add();
    visited.push_back(here.get().function_name());

    // Stop at the first failing check and hand its error to the caller.
    step_result range = check_range(value);
    if (range.is_ok() == false)
    {
        return range.unwrap_err();
    }

    step_result even = check_even(value);
    if (even.is_ok() == false)
    {
        return even.unwrap_err();
    }

    return {};
}

static void run(hsd::i32 value)
{
    hsd::stack_trace here = hsd::stack_trace::add();
    visited.clear();

    step_result res = validate(value);
    hsd::print("value {}:"_fmt, value);

    for (hsd::usize i = 0; i < visited.size(); ++i)
    {
        hsd::print(" {}"_fmt, visited[i]);
    }

    if (res.is_ok())
    {
        hsd::println(" -> ok"_fmt);
    }
    else
    {
        // pretty_error() dumps the live stack to stderr, then returns the message.
        // Stderr is not part of the checked output.
        hsd::stack_trace_error err = res.unwrap_err();
        hsd::println(" -> failed{}"_fmt, err.pretty_error());
    }
}

int main()
{
    run(42);
    run(7);
    run(250);

    return 0;
}
