#include "../../cpp/String.hpp"

#include <string>
#include <benchmark/benchmark.h>

void hsdString(benchmark::State& state)
{
    for(auto _ : state)
    {
        hsd::u8string str = "this is a test";
        auto str2 = str;
        hsd::u8string::to_string(-4.12f);
        (",\nno it\'s not\n" + str).data();
        str2.data();
        str.find('z');
        benchmark::DoNotOptimize(str.data());
    }
}

void stdString(benchmark::State& state)
{
    for(auto _ : state)
    {
        std::string str = "this is a test";
        auto str2 = str;
        std::to_string(-4.12f);
        (",\nno it\'s not\n" + str).data();
        str2.data();
        str.find('z');
        benchmark::DoNotOptimize(str.data());
    }
}

BENCHMARK(hsdString);

BENCHMARK(stdString);

BENCHMARK_MAIN();