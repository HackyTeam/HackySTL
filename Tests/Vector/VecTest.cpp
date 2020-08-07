#include <stdio.h>
#include "../../cpp/Vector.hpp"

struct S
{
    int a;
    float b;
    char c;
};

int main()
{
    hsd::vector e = {1, 2, 3, 4, 5, 6};

    for (auto val : e)
    {
        printf("%d\n", val);
    }

    puts("");
    auto e2 = e.gen_range<1, 3>();

    for (auto val : e2)
    {
        printf("%d\n", val);
    }

    hsd::vector<S> ls;
    ls.emplace_back(12, 3.3f, 'a');
}