#include <Time.hpp>
#include <stdio.h>

struct S
{
    int x, y;
    float f;

    struct {
        int i;
    } t;
};

void func(struct S* s) { __builtin_dump_struct(s, &printf); }

int main()
{
    using namespace hsd::time_literals;
    constexpr auto time_val = "28.03.2021 13:43:53"_dmy;

    constexpr auto some_val = S{100, 42, 3.1415, {12}};
    
    constexpr auto lambda = [=]<char... chars> {
        __builtin_dump_struct(&some_val, &printf);
        __builtin_dump_struct(&time_val, &printf);
    };

    __builtin_dump_struct(&lambda, &printf);
}