#include "../../cpp/Logging.hpp"

static void test_trace(hsd::stack_trace tr, const char* msg, hsd::usize N)
{
    if(N == 0)
        throw hsd::stack_trace_exception();

    invoke_stacktrace_func(test_trace, msg, N - 1);
}

static void test_profiler(hsd::profiler tr, hsd::usize N)
{
    if(N == 0)
        return;

    invoke_profiler_func(test_profiler, N - 1);
}

int main()
{
    try
    {
        invoke_stacktrace_func(test_trace, "hello", 32);   
    }
    catch(const std::exception& e)
    {
        hsd::io::err_print<"{}\n\n">(e.what());
    }

    invoke_profiler_func(test_profiler, 32);
}