#include <cassert>

#include <Containers/Bitset.hpp>

// Regression test: assigning one bit proxy to another writes the bit value.
int main()
{
    hsd::bitset<8> bits{0};
    bits[2] = bits[1] = bits[0] = true;
    assert(bits[0] && bits[1] && bits[2]);
    assert(bits.count() == 3);

    bits[5] = bits[0];
    assert(bits[5]);

    bits[0] = bits[7];
    assert(!bits[0]);
    assert(bits[1] && bits[2] && bits[5]);
    assert(!bits[7]);
}
