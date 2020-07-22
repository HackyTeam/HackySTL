#include <stdio.h>
#include "../cpp/Functional/Functional.hpp"

static constexpr int func(int a) noexcept
{
    return a;
}

int main()
{
    //function f = [](){return 4;};
    hsd::function f = func;
    f.bind(5);
    printf("%d\n", f());
}