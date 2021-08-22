#include <Functional.hpp>

// Implementations for "defaultarg.cpp"
static hsd::i32 func() noexcept 
{
    return 4;
}

extern "C" hsd::i32 bop(hsd::i32 a, hsd::i32 b) noexcept 
{
    return a * b;
};

extern "C" hsd::function<hsd::i32()> f(func);
