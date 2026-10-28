#include "Base/Utility.hpp"
#include "Containers/String.hpp"
#include "Containers/Vector.hpp"
#include "Wrappers/Tuple.hpp"
#include <Threading/Time.hpp>
#include <stdio.h>

struct S
{
    int x, y;
    float f;

    struct {
        int i;
    } t;
};

struct A
{
private:
    struct S s;

public:
    constexpr A(S v) : s{v}
    {}
};

//void func(struct S* s) { __builtin_dump_struct(s, &printf); }

template <typename  T>
hsd::decay_t<T> copy(T&& val)
{
    return val;
}

hsd::string str = "";
hsd::tuple<> tup;

template <typename... Args>
void test_f(const char* s, Args&&... args)
{
    str += s;
    //tup = tup + hsd::make_tuple(copy<Args>(args)...);
}

template <typename T, hsd::usize N>
static consteval auto v()
{
    __builtin_dump_struct(&test, &test_f);
}

int main()
{
    using namespace hsd::time_literals;
    constexpr auto time_val = "28.03.2021 13:43:53"_dmy;

    static constexpr auto some_val = S{100, 42, 3.1415, {12}};
    constexpr auto test = A{some_val};
    
    constexpr auto lambda = [=]<char... chars> {
        __builtin_dump_struct(&some_val, &printf);
        __builtin_dump_struct(&time_val, &printf);
        __builtin_dump_struct(&test, &printf);
    };

    lambda();
    __builtin_dump_struct(&test, &test_f);

    //constexpr const S *a = &some_val, *b = nullptr;
    constexpr auto val = v<S, 2>();

    //printf("%ld\n", v());

    auto s = str;
    //__builtin_dump_struct(&lambda, &printf);
}