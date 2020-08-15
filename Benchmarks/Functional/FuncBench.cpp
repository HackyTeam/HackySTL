#include "../../cpp/Functional.hpp"

#include <benchmark/benchmark.h>
#include <functional>

static constexpr int func(int a) noexcept
{
    return a;
}

static void hsdFunc(benchmark::State& state)
{
    for(auto _ : state)
    {
        int b = 5;
        hsd::function f = func;
        f(2); 
        auto f2 = f;
        f2(23);
        hsd::function f3 = hsd::bind(f, 5);
        f3();
    }
}

static void stdFunc(benchmark::State& state)
{
    for(auto _ : state)
    {
        int b = 5;
        std::function f = func;  
        f(2);
        auto f2 = f;
        f2(23);
        std::function<int()> f3 = std::bind(f, 5);
        f3();
    }
}

BENCHMARK(hsdFunc);

BENCHMARK(stdFunc);

BENCHMARK_MAIN();