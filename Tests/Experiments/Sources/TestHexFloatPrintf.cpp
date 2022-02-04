#include <stdio.h>

int main()
{
    printf("%a\n", 3.1415f);
    printf("%la\n", 3.1415);
    printf("%La\n", 3.1415l);

    int *a;
    decltype(*a) b;
}