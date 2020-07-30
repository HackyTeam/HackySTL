#include "../../cpp/Io/Io.hpp"

int main()
{
    hsd::io::print("hello, {} and other words\n", 123.2f);
    auto a = hsd::io::read().parse<size_t>();
    hsd::io::print("{}\n", a);
}