#include <Extra/CompressedPair.hpp>
#include <Wrappers/Pair.hpp>
#include <cassert>

struct empty {};

// constexpr implies inline
static constexpr empty empty_v{};

int main()
{
    hsd::compressed_pair<int, float> a(3, 4.f);
    hsd::compressed_pair<int, float> b(3, 4);

    hsd::compressed_pair<int, empty> c(1, empty_v);
    hsd::compressed_pair<empty, int> d(empty_v, 1);
    hsd::compressed_pair<empty, empty> e;

    static_assert(sizeof(a) == sizeof(hsd::pair<int, float>));
    static_assert(sizeof(c) == sizeof(int));
    static_assert(sizeof(d) == sizeof(int));
    static_assert(sizeof(e) == sizeof(empty) * 2);
    // it fails because e has a size of 1
    // there is no such thing as sizeof(something) == 0

    assert(a.first() == 3 && a.second() == 4.f);
    assert(b.first() == 3 && b.second() == 4.f);
    assert(c.first() == 1);
    assert(d.second() == 1);

    a.first() = 9;
    a.second() = 2.5f;
    assert(a.get<0>() == 9 && a.get<1>() == 2.5f);

    // Compile-time error
    //a.get<2>();
}