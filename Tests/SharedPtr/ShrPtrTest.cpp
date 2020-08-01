#include <iostream>
#include "../../cpp/SharedPtr.hpp"

using namespace hsd::non_atomic_types;

struct S
{
    int a;
    char b;
    float c;
    const char* d;
};

template<typename T>
static void print_val(T& val)
{
    std::cout << val << '\n';
}

static void print(shared_ptr<S> ptr)
{
    print_val(ptr->a);
    print_val(ptr->b);
    print_val(ptr->c);
    print_val(ptr->d);
}

int main()
{
    shared_ptr elm = shared_ptr<S>::make_shared(12, 'c', 4.3f, "str");
    auto elm2 = elm;
    print(elm);
    print(elm2);
}