//#include "../Functional/Functional.hpp"
#include <functional>

// Implementations for "defaultarg.cpp"
static int func() noexcept {
    return 4;
}
extern "C" int bop(int a, int b) noexcept {
    return a*b;
};
extern "C" std::function<int()> f(func);
