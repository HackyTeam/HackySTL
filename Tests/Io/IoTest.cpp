#include "../../cpp/Io/Io.hpp"

int main()
{
    int x, z;
    float y;
    hsd::io::print("hello, {} and other words\n", 123.2f);
    hsd::io::read_line().set_data(x, y, z);
    hsd::io::print("{}, {}, {}\n", x, y, z);
    auto file = hsd::file("test.txt", hsd::file::options::text::read);
    auto b = file.read().parse<int>();
    auto e = file.read_line().to_string();
    hsd::io::print("{}\n", b);
}