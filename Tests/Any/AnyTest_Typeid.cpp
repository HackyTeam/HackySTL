#include <iostream>

#define HSD_ANY_ENABLE_TYPEINFO
#include "../../cpp/Any.hpp"

struct S
{
    int _a;
    char _b;
    float _c;
    const char* _d;

    S(int a, char b, float c, const char* d) : _a(a), _b(b), _c(c), _d(d) {}
};

int main()
{
    std::boolalpha(std::cout);

    // any type
    hsd::any a = 1;
    std::cout << a.type().name() << ": " << a.cast_to<int>().unwrap() << '\n';
    a = 3.14;
    std::cout << a.type().name() << ": " << a.cast_to<double>().unwrap() << '\n';
    a = true;
    std::cout << a.type().name() << ": " << a.cast_to<bool>().unwrap() << '\n';
 
    // bad cast
    a = 1;
    std::cout << a.cast_to<float>().unwrap() << '\n';
 
    // has value
    a = 1;
    if (a.has_value())
    {
        std::cout << a.type().name() << '\n';
    }
 
    // reset
    a.reset();
    if (!a.has_value())
    {
        std::cout << "no value\n";
    }
 
    // pointer to contained data
    a = 1;
    if (int* i = a.cast_if<int>())
        std::cout << *i << "\n";

    // holds a given type
    a.emplace<S>(1, 'c', 4.2f, "str");
    if (a.holds_type<S>())
        std::cout << "Holds a S\n";
    
    if (a.holds_type<int>())
        std::cout << "Holds an int\n";
}