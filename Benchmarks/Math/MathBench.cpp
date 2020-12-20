#include <benchmark/benchmark.h>
#include <cmath>
#include "../../cpp/Math.hpp"

static void hsdSin(benchmark::State& state)
{
    float val = 0.f;

    for(auto _ : state)
    {
        float rez = hsd::math::sin(val);
        benchmark::DoNotOptimize(rez);
        val += 0.001f;
    }
}

static void stdSin(benchmark::State& state)
{
    float val = 0.f;

    for(auto _ : state)
    {
        float rez = std::sin(val);
        benchmark::DoNotOptimize(rez);
        val += 0.001f;
    }
}

BENCHMARK(hsdSin);

BENCHMARK(stdSin);

BENCHMARK_MAIN();