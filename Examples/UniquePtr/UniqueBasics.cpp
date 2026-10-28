// Title: Unique pointer basics
// Level: Basic
// Shows: make_unique, move-only ownership, assigning null, and arrays owned by a unique_ptr.

#include <Serialize/Io.hpp>
#include <Wrappers/UniquePtr.hpp>

using namespace hsd::format_literals;

// Announces its life cycle, so ownership changes are visible in the output.
struct noisy
{
    hsd::i32 id;

    explicit noisy(hsd::i32 value = 0)
        : id{value}
    {
        hsd::println("  create {}"_fmt, id);
    }

    ~noisy()
    {
        hsd::println("  destroy {}"_fmt, id);
    }
};

static const char* yes_no(bool value)
{
    return value ? "yes" : "no";
}

int main()
{
    hsd::println("single object:"_fmt);
    {
        // make_unique allocates the object and returns its only owner.
        auto first = hsd::make_unique<noisy>(1);
        hsd::println("first id: {}"_fmt, first->id);

        // Copying is not allowed. Moving transfers ownership and empties the source.
        hsd::unique_ptr<noisy> second{hsd::move(first)};
        hsd::println("first empty: {}, second id: {}"_fmt, yes_no(first == nullptr), second->id);

        // Assigning a new owner destroys the old object.
        second = hsd::make_unique<noisy>(2);
        hsd::println("second now owns {}"_fmt, second->id);

        // Assigning nullptr destroys the object right away.
        second = nullptr;
        hsd::println("second empty: {}"_fmt, yes_no(second == nullptr));
    }

    hsd::println("array:"_fmt);
    {
        // The array form default-constructs every element and destroys them all.
        auto values = hsd::make_unique<noisy[]>(2);
        values.get()[0].id = 10;
        values.get()[1].id = 11;
        hsd::println("ids: {} {}"_fmt, values.get()[0].id, values.get()[1].id);
    }

    hsd::println("plain value:"_fmt);
    auto number = hsd::make_unique<hsd::i32>(41);
    *number += 1;
    hsd::println("number: {}"_fmt, *number);
    return 0;
}
