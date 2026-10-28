#include <cassert>

#include <Containers/String.hpp>
#include <Wrappers/SharedPtr.hpp>

// Regression tests: the shared factories take constructor arguments that are
// lvalues, and an allocator still selects the allocator overload.
namespace
{
    struct package
    {
        hsd::string name;

        explicit package(const char* text)
            : name{text}
        {}
    };
}

int main()
{
    const char* label = "box";

    auto unsafe = hsd::make_unsafe_shared<package>(label);
    assert(unsafe->name.length() == 3);

    auto safe = hsd::make_safe_shared<package>(label);
    assert(safe->name.length() == 3);

    hsd::allocator alloc{};
    auto counted = hsd::make_unsafe_shared<hsd::i32>(alloc);
    assert(*counted == 0);
    auto atomic_counted = hsd::make_safe_shared<hsd::i32>(alloc);
    assert(*atomic_counted == 0);
}
