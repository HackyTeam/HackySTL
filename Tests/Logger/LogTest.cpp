#include <Logging.hpp>

template <hsd::usize N>
auto test_trace(hsd::stack_trace, const char* msg)
    -> hsd::option_err<hsd::stack_trace_error>
{
    invoke_stacktrace_func(test_trace<N - 1>, msg).unwrap();
    return {};
}

template <>
auto test_trace<0>(hsd::stack_trace, const char*)
    -> hsd::option_err<hsd::stack_trace_error>
{
    return hsd::stack_trace_error{};
}

template <hsd::usize N>
static void test_profiler(hsd::profiler)
{
    if constexpr (N > 0)
    {
        invoke_profiler_func(test_profiler<N - 1>);
    }
}

int main()
{
    //invoke_stacktrace_func(test_trace<120>, "hello").unwrap();   
    invoke_profiler_func(test_profiler<32>);
    return 0;
}