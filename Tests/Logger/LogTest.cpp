#include "../../cpp/Logging.hpp"

void test(const char* msg, size_t N = 0)
{
    if(N == 33)
        throw hsd::stack_trace_exception();

    HSD_STACKTRACE_FUNC(test, msg, N + 1);
}

int main()
{
    HSD_STACKTRACE_FUNC(test, "hello");
}