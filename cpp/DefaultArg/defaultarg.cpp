#include "defaultarg.hpp"
#include <functional>

#include <iostream>

// Pls -O3

int add(int a, int b) {
    return a+b;
}

// see: "extern_impl.cpp"
extern "C" int bop(int a, int b) noexcept;
extern "C" std::function<int()> f;

//static auto bopper = make_defaultcall<int(int, int)>(bop, 3, 4);
//static auto funcer = make_defaultcall<int()>(f);
defaultcall_t adder(&add, 3, 4);
static auto bopper = defaultcall_t(bop, 3, 4);
static auto funcer = defaultcall_t(f);

int main() {
    std::tuple t = {1, 42.3, "ara"};
    std::cout << bopper(default_v, default_v) << std::endl;
    std::cout << bopper(default_v, 1) << std::endl;
    std::cout << bopper(2, default_v) << std::endl;
    std::cout << bopper(2, 1) << std::endl;

    std::cout << adder(5, default_v) << std::endl;

    std::cout << funcer() << std::endl;
}