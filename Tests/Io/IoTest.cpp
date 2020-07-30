#include "../../cpp/Io/Io.hpp"

int main()
{
    hsd::io::print("hello, {} and other words\n", 123.2f);
    auto a = hsd::io::read_line();
    hsd::io::print("{}\n", a);
    auto file = hsd::file("test.txt", hsd::file::options::text::read);
    auto b = file.read().parse<int>();
    hsd::io::print("{}\n", b);
}