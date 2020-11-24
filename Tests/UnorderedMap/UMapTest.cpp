#include "../../cpp/UnorderedMap.hpp"
#include "../../cpp/HeapArray.hpp"
#include <stdio.h>

template <typename T>
using hsdCTAlloc = hsd::constexpr_allocator<T, 20>;

#if defined(HSD_COMPILER_CLANG)

constexpr auto gen_map()
{
    hsd::unordered_map<int, int, hsd::fnv1a<size_t>, hsdCTAlloc > map{};

    map.emplace(1, 0);
    map.emplace(2, 2);

    return map;
}

#endif

int main()
{
    hsd::heap_array<hsd::uchar, 4096> buf{};

    hsd::buffered_allocator<hsd::uchar> alloc{buf.data(), 4096};
    hsd::unordered_map<const char*, int, hsd::fnv1a<size_t>, hsd::buffered_allocator> map{alloc};

    map.emplace("key", 0);        
    map.emplace("key2", 1);
    map.emplace("key3", 2);
    map.emplace("key4", 0);
    map.emplace("key5", 1);
    map.emplace("key6", 2);
    map.emplace("key7", 0);
    map.emplace("key8", 1);

    printf("%d\n========\n", map["key8"]);
    map["key9"] = 2;
    printf("%d\n========\n", map["key9"]);


    for(auto& _it : map)
        printf("%s\n", _it.first);

    #if defined(HSD_COMPILER_CLANG)
    constexpr auto map2 = gen_map();

    printf("========\n%d\n========\n", map2[2]);

    for(auto& _it : map2)
        printf("%d\n", _it.first);
    #endif
}