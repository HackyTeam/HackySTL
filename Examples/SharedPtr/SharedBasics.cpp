// Title: Shared pointer basics
// Level: Basic
// Shows: unsafe_shared_ptr and safe_shared_ptr with copies, get_count, is_unique and reset by nullptr.

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/SharedPtr.hpp>

using namespace hsd::format_literals;

struct resource
{
    hsd::i32 id;

    explicit resource(hsd::i32 value)
        : id{value}
    {
        hsd::println("  create {}"_fmt, id);
    }

    ~resource()
    {
        hsd::println("  destroy {}"_fmt, id);
    }
};

// Both owner types have the same interface, so one template shows both.
// unsafe_shared_ptr counts with plain integers: fine inside one thread.
// safe_shared_ptr counts atomically: needed when owners live in several threads.
template <typename Owner>
static void show_counts(Owner first)
{
    hsd::println("single owner: count {} unique {}"_fmt,
        first.get_count(), first.is_unique() ? "yes" : "no");

    {
        // Copying shares the object and bumps the count.
        Owner second = first;
        hsd::println("with a copy: count {} unique {}"_fmt,
            first.get_count(), first.is_unique() ? "yes" : "no");
        hsd::println("same object: {}"_fmt, first.get() == second.get() ? "yes" : "no");
    }

    // The copy went out of scope, so the count dropped again.
    hsd::println("copy gone: count {}"_fmt, first.get_count());
}

int main()
{
    hsd::println("unsafe:"_fmt);
    show_counts(hsd::make_unsafe_shared<resource>(1));
    hsd::println("safe:"_fmt);
    show_counts(hsd::make_safe_shared<resource>(2));

    // The object lives until the last owner lets go, no matter which owner that is.
    hsd::println("last owner wins:"_fmt);
    auto keep = hsd::make_safe_shared<resource>(3);
    auto other = keep;
    keep = nullptr;
    hsd::println("keep is empty: {}, other count: {}"_fmt,
        keep == nullptr ? "yes" : "no", other.get_count());
    other = nullptr;
    hsd::println("done"_fmt);
    // The shared factories take lvalue constructor arguments.
    const char* label = "box";
    auto labeled = hsd::make_safe_shared<hsd::string>(label);
    hsd::println("labeled: {} ({} characters)"_fmt, *labeled, labeled->length());
    return 0;
}
