#include "../../cpp/Logging.hpp"
#include "../../cpp/Io.hpp"

#include <stdexcept>

static hsd::logger stack;

void test(const char* msg, size_t N = 0, hsd::logger& log = stack.add())
{
    if(N == 33)
    {   
        throw std::runtime_error(msg);
    }

    test(msg, N + 1);
}

int main()
{
    try
    {
        test("hello");
    }
    catch(std::exception& e)
    {
        for(auto& info : stack)
        {
            hsd::io::print("info: {}:{} Function: {}\n", 
                info.file_name(), info.line(), info.function_name());
        }
    }
}