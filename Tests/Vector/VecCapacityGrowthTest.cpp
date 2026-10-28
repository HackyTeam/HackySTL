#include <Containers/Vector.hpp>
#include <cassert>

int main()
{
    // Growth doubles, so pushing from empty visits 1, 2, 4, ... in order.
    hsd::vector<int> values;
    hsd::usize previous = values.capacity();
    for (int value = 0; value < 1000; ++value)
    {
        values.push_back(value);

        if (values.capacity() != previous)
        {
            assert(previous == 0
                ? values.capacity() == 1
                : values.capacity() == 2 * previous);
            previous = values.capacity();
        }
    }
    assert(values.capacity() == 1024);

    // A reserve or resize that needs more than one doubling still lands on
    // the first power of two that is large enough.
    hsd::vector<int> reserved;
    reserved.reserve(5);
    assert(reserved.capacity() == 8);

    hsd::vector<int> resized;
    resized.reserve(4);
    resized.resize(9);
    assert(resized.capacity() == 16);

    // A reserve that is already satisfied leaves the capacity unchanged.
    resized.reserve(3);
    assert(resized.capacity() == 16);

    return 0;
}
