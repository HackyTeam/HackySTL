#include <Time.hpp>
#include <Io.hpp>
#include <iostream>

int main()
{
    using namespace hsd::format_literals;

    hsd::precise_clock c1;

    hsd::io::cout().get_stream().reserve(4096);
    const hsd::u32 iterations = 1'000'000;
    
    c1.restart();

    for (hsd::u32 i = 0; i < iterations; ++i)
    {
        hsd::io::cout().get_stream().write_data<"Hello World {} a {}\r\n">(123231u, 4.232f);
    }

    hsd::println(
        "hsd IO elapsed time: {}ns"_fmt,  
        c1.restart().to_nanoseconds() / iterations
    );
    
    c1.restart();

    for (hsd::u32 i = 0; i < iterations; i++)
    {
        fprintf(stdout, "Hello World %u a %f\r", 123231u, 4.232f);
    }

    hsd::println(
        "C IO elapsed time: {}ns"_fmt, 
        c1.restart().to_nanoseconds() / iterations
    );
    
    c1.restart();

    for (hsd::u32 i = 0; i < iterations; i++)
    {
        std::cout << "Hello World " << 123231u << " a " << 4.232f << '\r';
    }

    hsd::println(
        "std IO elapsed time: {}ns"_fmt, 
        c1.restart().to_nanoseconds() / iterations
    );
    
    c1.restart();

    return 0;
}