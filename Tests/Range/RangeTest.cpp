#include <stdio.h>
#include "../../cpp/Extra/Range.hpp"

int main()
{
    for(auto i : hsd::Range(0, 15))
    {
        printf("%ld\n", i);
    }
}