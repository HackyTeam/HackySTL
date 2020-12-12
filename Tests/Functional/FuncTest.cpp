#include <stdio.h>
#include "../../cpp/Functional.hpp"

static int func(int a)
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
    hsd::function<int()> f3;
    
    {
        hsd::function f = func;  
        auto f2 = f;
        f3 = hsd::bind(func, hsd::make_tuple(5));
    }

    printf("%d\n", f3().unwrap());
    hsd::function my_counter = counter(1);
    my_counter().unwrap();
}