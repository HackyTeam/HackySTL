#include <cassert>

#include <Wrappers/Variant.hpp>

// A variant whose alternatives cannot be default-constructed is left valueless by its default
// constructor. Visiting such a variant reports the missing value and never calls the visitor,
// for a visitor that returns a value and for one that returns nothing, on a variant that is
// not const and on one that is.
namespace
{
    struct needs_value
    {
        explicit needs_value(int v) : value{v} {}
        int value;
    };
}

int main()
{
    bool called = false;

    hsd::variant<needs_value> empty;
    auto value_result = empty.visit([&called](const auto& v)
    {
        called = true;
        return v.value;
    });
    assert(!value_result.is_ok());

    auto void_result = empty.visit([&called](const auto&)
    {
        called = true;
    });
    assert(!void_result.is_ok());

    const hsd::variant<needs_value> const_empty;
    auto const_value_result = const_empty.visit([&called](const auto& v)
    {
        called = true;
        return v.value;
    });
    assert(!const_value_result.is_ok());

    auto const_void_result = const_empty.visit([&called](const auto&)
    {
        called = true;
    });
    assert(!const_void_result.is_ok());

    assert(!called);
    return 0;
}
