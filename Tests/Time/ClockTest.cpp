#include "../../cpp/Time.hpp"
#include "../../cpp/Io.hpp"

#include <iostream>

int main()
{
    hsd::clock c1;
    hsd::io::print("Hello World\n");
    hsd::io::print("hsd IO elapsed time: {}\n", c1.restart().to_microseconds());
    c1.restart();
    std::cout << "Hello World\n";
    hsd::io::print("std IO elapsed time: {}\n", c1.restart().to_microseconds());
}