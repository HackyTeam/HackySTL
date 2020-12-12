#include "../../cpp/UnorderedMap.hpp"
#include "../../cpp/HeapArray.hpp"
#include <stdio.h>

template <typename T>
using hsdCTAlloc = hsd::constexpr_allocator<T, 20>;

#if defined(HSD_COMPILER_CLANG)

constexpr auto gen_map()
{
    hsd::unordered_map<const char*, int, hsd::fnv1a<size_t>, hsdCTAlloc > map;

    map.emplace("1", 0);
    map.emplace("2", 1);
    map.emplace("3", 2);
    map.emplace("4", 3);
    map.emplace("5", 4);
    map.emplace("6", 5);
    map.emplace("7", 6);
    map.emplace("8", 7);
    map.emplace("9", 8);

    return map;
}

#endif

int main()
{
    hsd::heap_array<hsd::uchar, 3072> buf{};
    hsd::buffered_allocator<hsd::uchar> alloc{buf.data(), 3072};
    hsd::unordered_map<int, int, hsd::fnv1a<size_t>, hsd::buffered_allocator> map{alloc};

    map.emplace(1, 1);        
    map.emplace(2, 2);
    map.emplace(3, 3);
    map.emplace(4, 4);
    map.emplace(5, 5);
    map.emplace(6, 6);
    map.emplace(7, 7);
    map.emplace(8, 8);
    map.emplace(9, 9);
    map.emplace(10, 10);
    map.emplace(11, 11);
    map.emplace(12, 12);
    map.emplace(13, 13);
    map.emplace(14, 14);

    printf("%d\n========\n", map[8]);
    printf("%d\n========\n", map[9]);

    for(auto& _it : map)
        printf("%d\n", _it.first);

    #if defined(HSD_COMPILER_CLANG)
    constexpr auto map2 = gen_map();

    printf("========\n%d\n========\n", map2["2"]);

    for(auto& _it : map2)
        printf("%s\n", _it.first);
    #endif
}