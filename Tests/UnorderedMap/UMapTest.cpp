#include <UnorderedMap.hpp>
#include <HeapArray.hpp>
#include <stdio.h>

static auto check_erase()
{
    hsd::unordered_map<hsd::i32, hsd::i32> map;

    for(hsd::i32 i = 0; i < 20; i++)
        map.emplace(i * i, i);

    for(auto iter = map.begin(); iter != map.end();)
    {
        if(iter->second == 12)
        {
            iter = map.erase(iter).unwrap();
        }
        else
        {
            printf("%d %d\n", iter->first, iter->second);
            iter++;
        }
    }

    return map;
}

int main()
{
    hsd::heap_array<hsd::uchar, 4096> buf{};
    hsd::buffered_umap<hsd::i32, hsd::i32> map{{buf.data(), buf.size()}};

    check_erase();
    puts("============");

    for(hsd::i32 i = 1; i <= 14; i++)
        map.emplace(i, i);

    printf("%d\n========\n", map[8]);
    printf("%d\n========\n", map[9]);

    for(auto& _it : map)
        printf("%d\n", _it.first);
}