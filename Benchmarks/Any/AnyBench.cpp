#include "../../cpp/Any.hpp"

#include <benchmark/benchmark.h>
#include <any>

struct S
{
    int a;
    char b;
    float c;
    const char* d;
};

static void hsdAny(benchmark::State& state)
{
    for(auto _ : state)
    {
        // any type
        hsd::any a = 1;
        a.type().name(); 
        a.cast<int>();

        a = 3.14;
        a.type().name();
        a.cast<double>();

        a = true;
        a.type().name();
        a.cast<bool>();
    
        // bad cast
        try
        {
            a = 1;
            a.cast<float>();
        }
        catch (const hsd::bad_any_cast& e)
        {
            e.what();
        }
    
        // has value
        a = 1;
        if (a.has_value())
        {
            a.type().name();
        }
    
        // reset
        a.reset();
        if (!a.has_value()) {}
    }
}

static void stdAny(benchmark::State& state)
{
    for(auto _ : state)
    {
        // any type
        std::any a = 1;
        a.type().name(); 
        std::any_cast<int>(a);

        a = 3.14;
        a.type().name();
        std::any_cast<double>(a);

        a = true;
        a.type().name();
        std::any_cast<bool>(a);
    
        // bad cast
        try
        {
            a = 1;
            std::any_cast<float>(a);
        }
        catch (const std::bad_any_cast& e)
        {
            e.what();
        }
    
        // has value
        a = 1;
        if (a.has_value())
        {
            a.type().name();
        }
    
        // reset
        a.reset();
        if (!a.has_value()) {}
    }
}

BENCHMARK(hsdAny);

BENCHMARK(stdAny);

BENCHMARK_MAIN();