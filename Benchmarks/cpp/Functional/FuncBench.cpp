#include "../../cpp/Functional.hpp"

#include <benchmark/benchmark.h>
#include <stdio.h>
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
        printf("%d\r", f(2));
        auto f2 = [](int a){return a + 7;};
        printf("%d\r", f2(23));
        hsd::function f3 = hsd::bind(f, 5);
        printf("%d\r", f3());
    }
}

static void stdFunc(benchmark::State& state)
{
    for(auto _ : state)
    {
        int b = 5;
        std::function f = func;  
        printf("%d\r", f(2));
        auto f2 = [](int a){return a + 7;};
        printf("%d\r", f2(23));
        std::function<int()> f3 = std::bind(f, 5);
        printf("%d\r", f3());
    }
}

BENCHMARK(hsdFunc);

BENCHMARK(stdFunc);

BENCHMARK_MAIN();