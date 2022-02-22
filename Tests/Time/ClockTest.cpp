#include <Time.hpp>
#include <Io.hpp>
#include <iostream>

int main()
{
    hsd::precise_clock c1;
    const hsd::u32 iterations = 1'000'000;
    c1.restart();

    for (hsd::u32 i = 0; i < iterations; i++)
        hsd_print("Hello World {} a {}\r", 123231u, 4.232f);

    hsd_println(
        "hsd IO elapsed time: {}ns",  
        c1.restart().to_nanoseconds() / iterations
    );
    
    c1.restart();

    for (hsd::u32 i = 0; i < iterations; i++)
        printf("Hello World %u a %f\r", 123231u, 4.232f);

    hsd_println(
        "C IO elapsed time: {}ns", 
        c1.restart().to_nanoseconds() / iterations
    );
    
    c1.restart();

    for (hsd::u32 i = 0; i < iterations; i++)
        std::cout << "Hello World " << 123231u << " a " << 4.232f << '\r';

    hsd_println(
        "std IO elapsed time: {}ns", 
        c1.restart().to_nanoseconds() / iterations
    );
    
    c1.restart();

    return 0;
}