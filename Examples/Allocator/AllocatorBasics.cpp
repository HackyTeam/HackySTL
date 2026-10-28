// Title: Allocator basics
// Level: Basic
// Shows: the allocator_traits life cycle of allocate, construct, destroy and deallocate.

#include <Base/Allocator.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    hsd::allocator alloc;
    using traits = hsd::allocator_traits<hsd::allocator, hsd::i32>;

    // allocate returns a result holding raw, uninitialized storage.
    hsd::i32* values = traits::allocate(alloc, 3).unwrap();

    // construct places an object into that storage, one slot at a time.
    for (hsd::i32 index = 0; index < 3; ++index)
    {
        traits::construct(alloc, values + index, (index + 1) * 10);
    }

    hsd::print("values:"_fmt);
    for (hsd::i32 index = 0; index < 3; ++index)
    {
        hsd::print(" {}"_fmt, values[index]);
    }
    hsd::println(""_fmt);

    // Objects are destroyed first, then the storage is handed back.
    for (hsd::i32 index = 0; index < 3; ++index)
    {
        traits::destroy(alloc, values + index);
    }
    traits::deallocate(alloc, values, 3).unwrap();
    hsd::println("released"_fmt);

    // Asking for zero elements is not an error, it just gives no storage.
    hsd::i32* nothing = traits::allocate(alloc, 0).unwrap();
    hsd::println("zero-sized request is null: {}"_fmt, nothing == nullptr ? "true" : "false");

    return 0;
}
