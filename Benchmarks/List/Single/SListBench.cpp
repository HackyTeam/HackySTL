#include "../../../cpp/ForwardList.hpp"

#include <benchmark/benchmark.h>
#include <forward_list>
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
        hsd::forward_list<S> ls;
        
        ls.emplace_front(12, 3.3f, 'a');
        ls.emplace_front(1, 3.2f, 'd');
        for(auto it : ls);
        
        ls.pop_front();
        for(auto it : ls);
        
        ls.emplace_front(3, 23.22f, 'e');
        for(auto it : ls);
    }
}

static void stdList(benchmark::State& state)
{
    for (auto _ : state)
    {
        std::forward_list<S> ls;
        
        ls.emplace_front(12, 3.3f, 'a');
        ls.emplace_front(1, 3.2f, 'd');
        for(auto it : ls);
        
        ls.pop_front();
        for(auto it : ls);
        
        ls.emplace_front(3, 23.22f, 'e');
        for(auto it : ls);
    }
}

BENCHMARK(hsdList);

BENCHMARK(stdList);

BENCHMARK_MAIN();