#include "../../cpp/DefaultArg/DefaultArg.hpp"

#include <functional>
#include <iostream>

// Pls -O3

int add(int a, int b) 
{
    return a+b;
}

// see: "extern_impl.cpp"
extern "C" int bop(int a, int b) noexcept;
extern "C" std::function<int()> f;

//static auto bopper = make_defaultcall<int(int, int)>(bop, 3, 4);
//static auto funcer = make_defaultcall<int()>(f);
hsd::defaultcall_t adder(&add, 3, 4);
static auto bopper = hsd::defaultcall_t(bop, 3, 4);
static auto funcer = hsd::defaultcall_t(f);

int main() 
{
    std::cout << bopper(hsd::default_v, hsd::default_v) << std::endl;
    std::cout << bopper(hsd::default_v, 1) << std::endl;
    std::cout << bopper(2, hsd::default_v) << std::endl;
    std::cout << bopper(2, 1) << std::endl;

    std::cout << adder(5, hsd::default_v) << std::endl;

    std::cout << funcer() << std::endl;
}