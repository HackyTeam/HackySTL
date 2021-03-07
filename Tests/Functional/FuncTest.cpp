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

    int call()
    {
        return 33;
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
        auto f22 = hsd::bind(&counter::call, b);
        printf("%d\n", f22());
    }

    std::placeholders::_1;
    printf("%d\n", f3().unwrap());
    hsd::function my_counter = counter(1);
    my_counter().unwrap();
}