#include "../../cpp/UnorderedMap.hpp"
#include <stdio.h>

int main()
{
    hsd::unordered_map map = {
        hsd::pair{"key", 0},
        hsd::pair{"key2", 1},
        hsd::pair{"key3", 2},
        hsd::pair{"key4", 0},
        hsd::pair{"key5", 1},
        hsd::pair{"key6", 2},
        hsd::pair{"key7", 0},
        hsd::pair{"key8", 1}
    };

    map["key8"];
    map["key9"] = 2;

    static_assert(hsd::is_function<decltype(main)>::value);

    for(auto _it : map)
        printf("%s\n", _it.first);
}