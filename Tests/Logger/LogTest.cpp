#include "../../cpp/Logging.hpp"

void test(const char* msg, size_t N, hsd::stack_trace tr = hsd::exec_stack.add())
{
    if(N == 0)
        throw hsd::stack_trace_exception();

    test(msg, N - 1);
}

int main()
{
    try
    {
        test("hello", 32);   
    }
    catch(const std::exception& e)
    {
        hsd::io::err_print("{}\n", e.what());
    }
}