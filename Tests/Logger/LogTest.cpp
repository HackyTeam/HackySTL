#include <Logging.hpp>

static auto test_trace(hsd::stack_trace tr, const char* msg, hsd::usize N)
    -> hsd::Result<void, hsd::stack_trace_error>
{
    if(N == 0)
        return hsd::stack_trace_error{};

    invoke_stacktrace_func(test_trace, msg, N - 1).unwrap();
    return {};
}

static void test_profiler(hsd::profiler tr, hsd::usize N)
{
    if(N == 0)
        return;

    invoke_profiler_func(test_profiler, N - 1);
}

int main()
{
    invoke_stacktrace_func(test_trace, "hello", 20).unwrap();   
    invoke_profiler_func(test_profiler, 32);
}