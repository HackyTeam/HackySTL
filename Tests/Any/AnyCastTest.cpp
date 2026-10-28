#include <Base/TypeTraits.hpp>
#include <Wrappers/Any.hpp>
#include <cassert>

struct payload
{
    int number;
};

// A non-const any gives mutable access through its casts.
static void test_mutable_casts()
{
    hsd::any value{payload{4}};

    static_assert(hsd::is_same<decltype(value.cast_if<payload>()), payload*>::value);
    value.cast_if<payload>()->number = 5;
    assert(value.cast_if<payload>()->number == 5);

    value.cast_to<payload>().unwrap().get().number = 6;
    assert(value.cast_if<payload>()->number == 6);

    assert(value.cast_if<int>() == nullptr);
    assert(!value.cast_to<int>().is_ok());
}

// A const any gives read-only access through its casts.
static void test_const_casts()
{
    const hsd::any value{payload{7}};

    static_assert(hsd::is_same<decltype(value.cast_if<payload>()), const payload*>::value);
    static_assert(hsd::is_same<decltype(value.cast_to<payload>().unwrap().get()), const payload&>::value);

    assert(value.cast_if<payload>()->number == 7);
    assert(value.cast_to<payload>().unwrap().get().number == 7);

    assert(value.cast_if<int>() == nullptr);
    assert(!value.cast_to<int>().is_ok());
}

int main()
{
    test_mutable_casts();
    test_const_casts();
    return 0;
}
