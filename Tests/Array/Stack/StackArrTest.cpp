#include <stdio.h>
#include <StackArray.hpp>

int main()
{
    hsd::stack_array sarr = {1, 2, 3, 4, 5};

    for(auto i : sarr)
    {
        printf("%d\n", i);
    }
}