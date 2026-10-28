// Title: Trace hook
// Level: Basic
// Shows: HSD_TRACE_SCOPE with a custom trace_sink hook that prints names and depth only.

// The macro is decided when the header is included, so define it first.
#define HSD_ENABLE_TRACING
#include <Serialize/Io.hpp>
#include <Serialize/Tracing.hpp>

using namespace hsd::format_literals;

// The default hook prints timings, which change on every run. This one prints
// only what is stable. A hook must not open a trace scope of its own.
static void print_event(const hsd::trace_event& event)
{
    hsd::println("{} depth {}"_fmt, event.function, event.depth);
}

static void leaf()
{
    HSD_TRACE_SCOPE();
}

static void middle()
{
    HSD_TRACE_SCOPE();
    leaf();
}

int main()
{
    hsd::trace_sink::set_hook(&print_event);

    // Each scope reports when its block ends, so the innermost one comes first.
    middle();

    // A scope can also guard a plain block inside a function.
    {
        HSD_TRACE_SCOPE();
        hsd::println("inside the block"_fmt);
    }

    // Events from the default hook wait in the output buffer until a flush.
    hsd::trace_sink::flush();

    return 0;
}
