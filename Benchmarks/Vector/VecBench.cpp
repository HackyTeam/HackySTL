#include "../../cpp/Vector.hpp"

#include <benchmark/benchmark.h>
#include <vector>
#include <iostream>

static void hsdVector(benchmark::State& state)
{
    for(auto _ : state)
    {
        hsd::vector e = {1, 2, 3, 4, 5, 6};

        for (auto& val : e)
            std::cout << val << '\r';

        e.pop_back();

        for (auto& val : e)
            std::cout << val << '\r';

        e.pop_back();
        e.pop_back();

        for (auto& val : e)
            std::cout << val << '\r';

        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);

        for (auto& val : e)
            std::cout << val << '\r';

        benchmark::DoNotOptimize(e.data());
    }
}

static void stdVector(benchmark::State& state)
{
    for(auto _ : state)
    {
        std::vector e = {1, 2, 3, 4, 5, 6};

        for (auto& val : e)
            std::cout << val << '\r';

        e.pop_back();

        for (auto& val : e)
            std::cout << val << '\r';

        e.pop_back();
        e.pop_back();

        for (auto& val : e)
            std::cout << val << '\r';

        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);
        e.emplace_back(5);

        for (auto& val : e)
            std::cout << val << '\r';

        benchmark::DoNotOptimize(e.data());
    }
}

BENCHMARK(hsdVector);

BENCHMARK(stdVector);

BENCHMARK_MAIN();