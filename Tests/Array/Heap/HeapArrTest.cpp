#include <stdio.h>
#include <HeapArray.hpp>

int main()
{
    hsd::heap_array harr = {{1, 2, 3, 4, 5}};
    int v = 33;
    printf("%d\n", harr.at(5).unwrap_or(v));
    
    for (auto i : harr)
    {
        printf("%d\n", i);
    }
}