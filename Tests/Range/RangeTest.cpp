#include <stdio.h>
#include "../cpp/Range/Range.hpp"

int main()
{
    for(auto i : hsd::Range(0, 15))
    {
        printf("%d\n", i);
    }
}