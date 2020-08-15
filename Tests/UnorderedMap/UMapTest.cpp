#include "../../cpp/UnorderedMap.hpp"
#include <stdio.h>

int main()
{
    hsd::unordered_map<const char*, int> map;
    map["key"] = 0;
    map["key2"] = 1;
    map["key3"] = 2;
    map["key4"] = 0;
    map["key5"] = 1;
    map["key6"] = 2;
    map["key7"] = 0;
    map["key8"] = 1;
    map["key9"] = 2;

    for(auto _it : map)
        printf("%s\n", _it.first);   
}