#include <cassert>
#include <cstdint>
#include <string_view>

#include <Base/Allocator.hpp>
#include <Support/AbortSupport.hpp>

// The failure and coalescing paths of the heap allocator and the buffered allocator,
// plus the allocator_traits wrappers, which are checked for a few allocator types.
// Buffers that are too small or missing end the process, so those are checked in a
// child process.

namespace
{
    // Over-aligned type, so allocation takes the alignment padding path.
    struct alignas(64) wide
    {
        char data[64];
    };

    template <typename T>
    bool is_error(hsd::result<T*, hsd::allocator_detail::allocator_error>& res,
        std::string_view message)
    {
        return !res.is_ok() && std::string_view{res.unwrap_err().pretty_error()} == message;
    }

    void check_heap_allocator()
    {
        // Zero bytes give a null pointer, and a null pointer deallocates as a no-op.
        auto _none = hsd::allocator::allocate<int>(0);
        assert(_none.is_ok() && _none.unwrap() == nullptr);
        assert(hsd::allocator::deallocate<int>(nullptr, 0).is_ok());

        // A length whose byte count overflows is rejected before any memory is requested.
        auto _too_long = hsd::allocator::allocate<int>(hsd::limits<hsd::usize>::max);
        assert(is_error(_too_long, "Bad length for allocation"));

        // A valid allocation is writable and can be freed.
        auto _block = hsd::allocator::allocate<int>(16);
        assert(_block.is_ok());
        int* _values = _block.unwrap();
        for (int index = 0; index < 16; ++index)
        {
            _values[index] = index * 3;
        }
        assert(_values[15] == 45);
        assert(hsd::allocator::deallocate<int>(_values, 16).is_ok());

        // Over-aligned types are placed on their alignment.
        auto _aligned = hsd::allocator::allocate<wide>(2);
        assert(_aligned.is_ok());
        assert(reinterpret_cast<std::uintptr_t>(_aligned.unwrap()) % alignof(wide) == 0);
        assert(hsd::allocator::deallocate<wide>(reinterpret_cast<wide*>(_aligned.unwrap()), 2).is_ok());

#if !defined(__SANITIZE_ADDRESS__) && !defined(__SANITIZE_THREAD__)
        // Sanitizers abort on requests they cannot satisfy, so this check runs only without them.
        // The request is too large for any allocator to satisfy.
        auto _huge = hsd::allocator::allocate<char>(hsd::limits<hsd::usize>::max / 4);
        assert(is_error(_huge, "No space left in RAM"));
#endif
    }

    void check_buffered_allocator()
    {
        alignas(64) hsd::uchar _storage[4096]{};
        hsd::buffered_allocator _buffer{_storage, sizeof(_storage)};

        // Zero bytes give a null pointer.
        auto _none = _buffer.allocate<int>(0);
        assert(_none.is_ok() && _none.unwrap() == nullptr);

        // A length whose byte count overflows is rejected before the buffer is searched.
        auto _overflowing = _buffer.allocate<int>(hsd::limits<hsd::usize>::max);
        assert(is_error(_overflowing, "Bad length for allocation"));

        // A request larger than the buffer reports insufficient memory.
        auto _too_big = _buffer.allocate<int>(sizeof(_storage));
        assert(is_error(_too_big, "Insufficient memory"));

        // Three blocks, released in an order that merges each freed block with its free
        // neighbours (next block first, then previous block).
        auto _first = _buffer.allocate<int>(8);
        auto _second = _buffer.allocate<int>(8);
        auto _third = _buffer.allocate<int>(8);
        assert(_first.is_ok() && _second.is_ok() && _third.is_ok());

        // Freeing the middle block first merges nothing, freeing the last merges with the middle,
        // and freeing the first merges with the free block that now follows it.
        assert(_buffer.deallocate(_second.unwrap(), 8).is_ok());
        assert(_buffer.deallocate(_third.unwrap(), 8).is_ok());
        assert(_buffer.deallocate(_first.unwrap(), 8).is_ok());

        // After the merges the whole buffer is free again, so a large block fits.
        auto _again = _buffer.allocate<int>(900);
        assert(_again.is_ok());

        // A block aligned beyond the header takes the padding path.
        auto _aligned = _buffer.allocate<wide>(4);
        assert(_aligned.is_ok());

        // A block that is freed in reverse order merges with the previous free block.
        auto _left = _buffer.allocate<int>(4);
        auto _right = _buffer.allocate<int>(4);
        assert(_left.is_ok() && _right.is_ok());
        assert(_buffer.deallocate(_right.unwrap(), 4).is_ok());
        assert(_buffer.deallocate(_left.unwrap(), 4).is_ok());

        // Pointers outside the buffer, and pointers inside it that were never returned.
        int outside = 0;
        auto _outside = _buffer.deallocate(&outside, 1);
        assert(!_outside.is_ok() && std::string_view{_outside.unwrap_err().pretty_error()} == "Pointer out of bounds");
        auto _inside = _buffer.allocate<int>(2);
        assert(_inside.is_ok());
        auto* _not_a_block = reinterpret_cast<int*>(_inside.unwrap()) + 1;
        assert(!_buffer.deallocate(_not_a_block, 1).is_ok());
        assert(_buffer.deallocate(_inside.unwrap(), 2).is_ok());
        assert(_buffer.deallocate<int>(nullptr, 0).is_ok());
    }

    void check_allocator_traits()
    {
        // The traits forward allocation, construction and destruction to the allocator.
        using heap_traits = hsd::allocator_traits<hsd::allocator, int>;
        hsd::allocator _heap{};
        auto _block = heap_traits::allocate(_heap, 4);
        assert(_block.is_ok());

        int* _values = _block.unwrap();
        heap_traits::construct(_heap, _values, 7);
        assert(*_values == 7);
        heap_traits::destroy(_heap, _values);
        assert(heap_traits::deallocate(_heap, _values, 4).is_ok());

        alignas(64) hsd::uchar _storage[512]{};
        hsd::buffered_allocator _buffer{_storage, sizeof(_storage)};
        using buffer_traits = hsd::allocator_traits<hsd::buffered_allocator, int>;
        auto _slot = buffer_traits::allocate(_buffer, 3);
        assert(_slot.is_ok());
        buffer_traits::construct(_buffer, _slot.unwrap(), 11);
        assert(*_slot.unwrap() == 11);
        buffer_traits::destroy(_buffer, _slot.unwrap());
        assert(buffer_traits::deallocate(_buffer, _slot.unwrap(), 3).is_ok());
    }
}

int main()
{
    check_heap_allocator();
    check_buffered_allocator();
    check_allocator_traits();

    // A null or too-small buffer is a programming error, and ends the process.
    test_support::expect_abort([]
    {
        hsd::buffered_allocator _bad{nullptr, 128};
        (void)_bad;
    });
    test_support::expect_abort([]
    {
        hsd::uchar _tiny[4]{};
        hsd::buffered_allocator _bad{_tiny, sizeof(_tiny)};
        (void)_bad;
    });
}
