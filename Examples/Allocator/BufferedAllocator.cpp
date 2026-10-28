// Title: Buffered allocator
// Level: Intermediate
// Shows: a vector living in a stack buffer, allocation failure, and block reuse.

#include <Base/Allocator.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    // The buffered allocator does not own memory: it carves blocks out of
    // the array it is given, so nothing here touches the heap.
    alignas(64) hsd::uchar bytes[4096]{};
    hsd::buffered_allocator alloc{bytes, sizeof(bytes)};

    {
        hsd::buffered_vector<hsd::i32> squares{alloc};

        for (hsd::i32 value = 1; value <= 8; ++value)
        {
            squares.push_back(value * value);
        }

        hsd::print("squares:"_fmt);
        for (hsd::usize index = 0; index < squares.size(); ++index)
        {
            hsd::print(" {}"_fmt, squares[index]);
        }
        hsd::println(""_fmt);
    }

    // A request larger than the buffer fails with an error instead of
    // aborting, as long as the caller uses the allocator directly.
    auto too_big = alloc.allocate<hsd::i32>(100000);
    hsd::println("huge request ok: {}"_fmt, too_big.is_ok() ? "true" : "false");
    if (!too_big.is_ok())
    {
        hsd::println("reason: {}"_fmt, too_big.unwrap_err().pretty_error());
    }

    // Freed blocks are merged back, so the next request reuses the same spot.
    hsd::i32* first = alloc.allocate<hsd::i32>(16).unwrap();
    alloc.deallocate(first, 16).unwrap();
    hsd::i32* second = alloc.allocate<hsd::i32>(16).unwrap();
    hsd::println("block reused: {}"_fmt, first == second ? "true" : "false");
    alloc.deallocate(second, 16).unwrap();

    // Giving back a pointer the allocator never handed out is reported too.
    hsd::i32 outside = 0;
    auto bad_free = alloc.deallocate(&outside, 1);
    hsd::println("foreign pointer rejected: {}"_fmt, bad_free.is_ok() ? "false" : "true");

    return 0;
}
