#include <iostream>
#include "../../cpp/Any/Any.hpp"

int main()
{
    // any type
    hsd::any a = 1;
    //std::cout << a.type().name() << ": " << a.cast<int>() << '\n';
    a = 3.14;
    //std::cout << a.type().name() << ": " << a.cast<double>() << '\n';
    a = true;
    //std::cout << a.type().name() << ": " << a.cast<bool>() << '\n';
 
    // bad cast
    try
    {
        a = 1;
        std::cout << a.cast<float>() << '\n';
    }
    catch (const hsd::bad_any_cast& e)
    {
        std::cout << e.what() << '\n';
    }
 
    // has value
    a = 1;
    if (a.has_value())
    {
        //std::cout << a.type().name() << '\n';
    }
 
    // reset
    a.reset();
    if (!a.has_value())
    {
        std::cout << "no value\n";
    }
 
    // pointer to contained data
    a = 1;
    int* i = a.cast<int*>();
    std::cout << *i << "\n";
}