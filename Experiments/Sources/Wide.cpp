#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <Unicode.hpp>

int main()
{
    const wchar_t *wstr = L"Hello, world! ©";
    char buf[256]{};

    hsd::unicode::length(wstr);
    printf("%s\n", buf);
    return 0;
}