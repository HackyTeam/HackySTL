#include "../../cpp/UnorderedMap.hpp"

#include <iostream>
#include <unordered_map>
#include <benchmark/benchmark.h>

auto gen_std_umap()
{
    std::unordered_map<const char*, int> map;
    map.emplace("key1", 0);
    map.emplace("key2", 1);
    map.emplace("key3", 2);
    map.emplace("key4", 0);
    map.emplace("key5", 1);
    map.emplace("key6", 2);
    map.emplace("key7", 0);
    map.emplace("key8", 1);

    return map;
}

constexpr auto gen_hsd_umap()
{
    hsd::static_umap<const char*, int, 10> map;
    map.emplace("key1", 0);
    map.emplace("key2", 1);
    map.emplace("key3", 2);
    map.emplace("key4", 0);
    map.emplace("key5", 1);
    map.emplace("key6", 2);
    map.emplace("key7", 0);
    map.emplace("key8", 1);

    return map;
}


static void hsdMap(benchmark::State& state)
{
    for(auto _ : state)
    {
        constexpr auto map = gen_hsd_umap();

        for(auto _it : map)
            std::cout << _it.second << '\r';

        benchmark::DoNotOptimize(map);
    }
}
    
static void stdMap(benchmark::State& state)
{
    for(auto _ : state)
    {
        const auto map = gen_std_umap();
    
        for(auto _it : map)
            std::cout << _it.second << '\r';

        benchmark::DoNotOptimize(map);
    }
}

BENCHMARK(hsdMap);

BENCHMARK(stdMap);

BENCHMARK_MAIN();