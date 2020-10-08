#include "../../cpp/Logging.hpp"

void test_trace(const char* msg, size_t N, hsd::stack_trace tr = hsd::exec_stack.add())
{
    if(N == 0)
        throw hsd::stack_trace_exception();

    test_trace(msg, N - 1);
}

void test_profiler(size_t N, hsd::profiler tr = hsd::profiler_stack.add())
{
    if(N == 0)
        return;

    test_profiler(N - 1);
}

int main()
{
    try
    {
        test_trace("hello", 32);   
    }
    catch(const std::exception& e)
    {
        hsd::io::err_print("{}\n\n", e.what());
    }

    test_profiler(32);
}