#include <Logging.hpp>
#include <stdio.h>

template <hsd::usize N>
auto test_trace(hsd::stack_trace tr, const char* msg)
    -> hsd::Result<void, hsd::stack_trace_error>
{
    invoke_stacktrace_func(test_trace<N - 1>, msg).unwrap();
    return {};
}

template <>
auto test_trace<0>(hsd::stack_trace tr, const char* msg)
    -> hsd::Result<void, hsd::stack_trace_error>
{
    return hsd::stack_trace_error{};
}

template <hsd::usize N>
static void test_profiler(hsd::profiler tr)
{
    invoke_profiler_func(test_profiler<N - 1>);
}

template <>
void test_profiler<0>(hsd::profiler tr) {}

int main()
{
    //invoke_stacktrace_func(test_trace<120>, "hello").unwrap();   
    invoke_profiler_func(test_profiler<32>);
}