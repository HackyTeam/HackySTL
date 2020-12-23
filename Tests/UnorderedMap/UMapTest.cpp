#include "../../cpp/UnorderedMap.hpp"
#include "../../cpp/HeapArray.hpp"
#include <stdio.h>

template <hsd::usize N>
constexpr auto gen_map()
{
    hsd::static_umap< hsd::i32, hsd::i32, N > map{};

    for(hsd::i32 i = 0; i < N; i++)
        map.emplace(i * i, i);

    return map;
}

int main()
{
    hsd::heap_array<hsd::uchar, 3072> buf{};
    hsd::buffered_umap<hsd::i32, hsd::i32> map{{buf.data(), 3072}};

    for(hsd::i32 i = 1; i <= 14; i++)
        map.emplace(i, i);

    printf("%d\n========\n", map[8]);
    printf("%d\n========\n", map[9]);

    for(auto& _it : map)
        printf("%d\n", _it.first);

    constexpr auto map2 = gen_map<10>();

    printf("========\n%d\n========\n", map2[49]);

    for(auto& _it : map2)
        printf("%d\n", _it.first);
}