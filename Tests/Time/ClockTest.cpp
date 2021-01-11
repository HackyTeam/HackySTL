#include <Time.hpp>
#include <Io.hpp>
#include <iostream>

int main()
{
    hsd::clock c1;
    hsd::clock::sleep_for(3.5f, []{});
    const hsd::u32 iterations = 4000000;
    c1.restart();

    for(hsd::u32 i = 0; i < iterations; i++)
        hsd::io::print<"Hello World {} a {}\r">(123231u, 4.232f);

    hsd::io::print<"hsd IO elapsed time: {}ns\n">( 
        c1.restart().to_microseconds() / (iterations / 1000));
    c1.restart();

    for(hsd::u32 i = 0; i < iterations; i++)
        printf("Hello World %u a %f\r", 123231u, 4.232f);

    hsd::io::print<"C IO elapsed time: {}ns\n">( 
        c1.restart().to_microseconds() / (iterations / 1000));
    c1.restart();

    for(hsd::u32 i = 0; i < iterations; i++)
        std::cout << "Hello World " << 123231u << " a " << 4.232f << '\r';

    hsd::io::print<"std IO elapsed time: {}ns\n">(
        c1.restart().to_microseconds() / (iterations/ 1000));
    c1.restart();

    return 0;
}