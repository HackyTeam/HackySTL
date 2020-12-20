#include <benchmark/benchmark.h>
#define FMT_HEADER_ONLY

#include <iostream>
#include <fmt/format.h>
#include "../../cpp/Io.hpp"

static void hsdIO(benchmark::State& state)
{
    for(auto _ : state)
    {
        hsd::io::print<"Hello World {} a {}\r">(123231u, 4.232f);
    }
}

static void stdIO(benchmark::State& state)
{
    for(auto _ : state)
    {
        std::cout << "Hello World " << 123231u << " " << "a" << " " << 4.232f << "\r";
    }
}

BENCHMARK(hsdIO);

BENCHMARK(stdIO);

BENCHMARK_MAIN();