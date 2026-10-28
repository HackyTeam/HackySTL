#include <Base/TypeTraits.hpp>
#include <Base/Utility.hpp>
#include <cassert>

// is_nothrow_constructible matches std::is_nothrow_constructible: the
// construction must exist and be declared noexcept.
struct throwing_ctor
{
    throwing_ctor(int) {}
};

struct quiet_ctor
{
    quiet_ctor(int) noexcept {}
};

static_assert(hsd::is_nothrow_constructible<int>::value);
static_assert(hsd::is_nothrow_constructible<int, int>::value);
static_assert(!hsd::is_nothrow_constructible<int, int*>::value);
static_assert(!hsd::is_nothrow_constructible<throwing_ctor, int>::value);
static_assert(hsd::is_nothrow_constructible<quiet_ctor, int>::value);
static_assert(hsd::is_nothrow_constructible<quiet_ctor, int&&>::value);
static_assert(!hsd::is_nothrow_constructible<throwing_ctor, int&&>::value);

struct Holder
{
    int value;
};

// launder can be used in constant evaluation on GCC and Clang.
constexpr int read_through_launder()
{
    int value = 1;
    return *hsd::launder(&value);
}
static_assert(read_through_launder() == 1);

// After the object in storage is replaced, launder yields the new object.
static void test_launder_replaced_object()
{
    alignas(Holder) unsigned char storage[sizeof(Holder)];

    Holder* first = hsd::construct_at(reinterpret_cast<Holder*>(storage), Holder{1});
    first->~Holder();

    Holder* second = hsd::construct_at(reinterpret_cast<Holder*>(storage), Holder{2});
    assert(hsd::launder(reinterpret_cast<Holder*>(storage))->value == 2);
    assert(hsd::launder(second)->value == 2);

    second->~Holder();
}

int main()
{
    test_launder_replaced_object();
    return 0;
}
