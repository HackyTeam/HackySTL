#include "../../../cpp/List.hpp"

#include <benchmark/benchmark.h>
#include <list>
#include <stdio.h>

struct S
{
    int a;
    float b;
    char c;
};

static void hsdList(benchmark::State& state)
{
    for (auto _ : state)
    {
        hsd::list<S> ls;
        
        ls.emplace_back(12, 3.3f, 'a');
        ls.emplace_back(1, 3.2f, 'd');
        for(auto it : ls);
        
        ls.pop_front();
        for(auto it : ls);
        
        ls.emplace_front(3, 23.22f, 'e');
        for(auto it : ls);
        
        ls.pop_back();
        for(auto it : ls);
    }
}

static void stdList(benchmark::State& state)
{
    for (auto _ : state)
    {
        std::list<S> ls;
        
        ls.emplace_back(12, 3.3f, 'a');
        ls.emplace_back(1, 3.2f, 'd');
        for(auto it : ls);
        
        ls.pop_front();
        for(auto it : ls);
        
        ls.emplace_front(3, 23.22f, 'e');
        for(auto it : ls);
        
        ls.pop_back();
        for(auto it : ls);
    }
}

BENCHMARK(hsdList);

BENCHMARK(stdList);

BENCHMARK_MAIN();