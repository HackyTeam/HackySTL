#include "../../cpp/Vector.hpp"

#include <benchmark/benchmark.h>
#include <vector>

static void hsdVector(benchmark::State& state)
{
    for(auto _ : state)
    {
        hsd::vector e = {1, 2, 3, 4, 5, 6};

        for (auto val : e);

        e.pop_back();

        for (auto val : e);

        e.pop_back();
        e.pop_back();

        for (auto val : e);

        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);

        for (auto val : e);
    }
}

static void stdVector(benchmark::State& state)
{
    for(auto _ : state)
    {
        std::vector e = {1, 2, 3, 4, 5, 6};

        for (auto val : e);

        e.pop_back();

        for (auto val : e);

        e.pop_back();
        e.pop_back();

        for (auto val : e);

        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);

        for (auto val : e);
    }
}

BENCHMARK(hsdVector);

BENCHMARK(stdVector);

BENCHMARK_MAIN();