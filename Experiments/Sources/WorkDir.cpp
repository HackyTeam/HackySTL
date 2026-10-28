#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main()
{
    const char* dir = get_current_dir_name();
    puts(dir);
    free((void*)dir);
}