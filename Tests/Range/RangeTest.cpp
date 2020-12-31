#include <stdio.h>
#include <Extra/Range.hpp>

int main()
{
    for(auto i : hsd::Range(0, 15))
    {
        printf("%ld\n", i);
    }
}