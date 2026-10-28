// Title: Call tree statistics
// Level: Intermediate
// Shows: tracing a recursive function and summarizing the events by depth in a hook.

#define HSD_ENABLE_TRACING
#include <Serialize/Io.hpp>
#include <Serialize/Tracing.hpp>

using namespace hsd::format_literals;

namespace
{
    constexpr hsd::usize max_depth = 8;

    // The sink calls the hook one event at a time under its lock, so plain
    // members are safe to update here.
    struct summary
    {
        hsd::usize total = 0;
        hsd::usize per_depth[max_depth] = {};
        hsd::u32 deepest = 0;
        bool print_each = false;
    };

    summary stats;

    void collect(const hsd::trace_event& event)
    {
        ++stats.total;
        ++stats.per_depth[event.depth];

        if (event.depth > stats.deepest)
        {
            stats.deepest = event.depth;
        }

        if (stats.print_each)
        {
            // Indent by depth to show how the calls nest. Events arrive in the
            // order the calls finish, so children come before their parent.
            for (hsd::u32 i = 0; i < event.depth; ++i)
            {
                hsd::print("  "_fmt);
            }

            hsd::println("{}"_fmt, event.function);
        }
    }

    hsd::u32 fib(hsd::u32 n)
    {
        HSD_TRACE_SCOPE();

        if (n < 2)
        {
            return n;
        }

        return fib(n - 1) + fib(n - 2);
    }

    void run(hsd::u32 n, bool print_each)
    {
        stats = summary{};
        stats.print_each = print_each;

        hsd::u32 result = fib(n);
        hsd::println("fib({}) = {}, calls = {}, deepest level = {}"_fmt,
            n, result, stats.total, stats.deepest);

        for (hsd::u32 level = 0; level <= stats.deepest; ++level)
        {
            hsd::println("  level {}: {} calls"_fmt, level, stats.per_depth[level]);
        }
    }
}

int main()
{
    hsd::trace_sink::set_hook(&collect);

    // Small input first: print every event so the nesting is visible.
    run(3, true);

    // Larger input: only the totals, because the tree grows quickly.
    run(6, false);

    return 0;
}
