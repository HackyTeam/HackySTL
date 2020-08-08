#include "../../cpp/UniquePtr.hpp"

#include <benchmark/benchmark.h>
#include <memory>

static void hsdUnique(benchmark::State& state)
{
    for(auto _ : state)
    {
        auto a = hsd::make_unique<int>(1);
        hsd::unique_ptr<int> b = hsd::move(a);
        hsd::unique_ptr<int[]> c = hsd::make_unique<int[]>(5);
    }
}

static void stdUnique(benchmark::State& state)
{
    for(auto _ : state)
    {
        auto a = std::make_unique<int>(1);
        std::unique_ptr<int> b = std::move(a);
        std::unique_ptr<int[]> c = std::make_unique<int[]>(5);
    }
}

BENCHMARK(hsdUnique);

BENCHMARK(stdUnique);

BENCHMARK_MAIN();