#include <stdio.h>

int main()
{
    int precision = 10;
    int biggerPrecision = 16;
    const char greetings[] = "Hello world";

    //printf("|%.8s|\n", greetings);
    //printf("|%.*s|\n", precision , greetings);
    //printf("|%16s|\n", greetings);
    //printf("|%*s|\n", biggerPrecision , greetings);

    char buf[30]{};

    for (auto i = 0; i < 29; ++i)
    {
        buf[i] = 'a';
    }

    auto sz = snprintf(buf, 30, "%d", 1242);
    sz += snprintf(buf + sz, 30 - sz, "%s", "raea");
    printf("|%.*s|", sz, buf);

    return 0;
}