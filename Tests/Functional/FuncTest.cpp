#include <stdio.h>
#include "../../cpp/Functional.hpp"

static constexpr int func(int a) noexcept
{
    return a;
}

struct counter {
    counter(int a = 0) : c(a) {}
    int c;
    int operator()()
    {
        return c++;
    }
};

int main()
{
    int b = 5;
    hsd::function f = func;  
    auto f2 = f;
    hsd::function f3 = hsd::bind(f, hsd::make_tuple(5));
    printf("%d\n", f3());
    hsd::function my_counter = counter(1);
    my_counter();
}