#include <stdlib.h>
#include <Types.hpp>
#include <Pair.hpp>

struct alloc_data
{
    hsd::usize _sz;
    char _values[];
};

int main()
{
    auto* data = reinterpret_cast<alloc_data*>(
        malloc(sizeof(alloc_data) + sizeof(hsd::i32) * 5)
    );

    data->_sz = 5;
    free(data);
}