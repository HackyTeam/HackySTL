#include <cstring>
#include <stdio.h>
#include <errno.h>

int main()
{
    int v;
    int res = sscanf("adsd", "%d", v);
    printf("%d %s\n", errno, strerror(errno));
}