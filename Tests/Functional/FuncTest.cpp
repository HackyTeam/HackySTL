#include <stdio.h>
#include <Functional.hpp>

static int func(int a)
{
    return a;
}

static void func2(auto& val)
{
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
    counter b = 5;
    hsd::function/*<void(counter&)>*/ val = func2<counter>;
    val(hsd::reference(b)).unwrap();

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