#include <stdio.h>
#include <InitializerList.hpp>

template< typename T >
static void some(hsd::initializer_list<T> l) {}

int main()
{
    hsd::initializer_list t = {1, 2, 3, 4};
    some(t);
    
    for(auto i : t)
    {
        printf("%d\n", i);
    }
}