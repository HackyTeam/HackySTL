#include <Base/Utility.hpp>
#include <cassert>

constexpr int construct_in_constant_evaluation()
{
    hsd::u32 storage[2]{};
    hsd::construct_at(&storage[1], 42u);
    return static_cast<int>(storage[1]);
}

static_assert(construct_in_constant_evaluation() == 42);

// Counts how many times the object is destroyed, through a counter the caller owns.
struct counted_destructor
{
    int* destroyed;

    constexpr ~counted_destructor()
    {
        ++*destroyed;
    }
};

// The union keeps the object's storage apart from its lifetime, so the test can end
// the lifetime explicitly with destroy_at.
union destroy_slot
{
    int unused;
    counted_destructor value;

    constexpr destroy_slot()
        : unused{0}
    {}

    constexpr ~destroy_slot()
    {}
};

constexpr int destroy_in_constant_evaluation()
{
    int destroyed = 0;
    destroy_slot slot;
    hsd::construct_at(&slot.value, &destroyed);
    hsd::destroy_at(&slot.value);
    return destroyed;
}

static_assert(destroy_in_constant_evaluation() == 1);

int main()
{
    // destroy_at runs the destructor of one object, and of each element of an array in
    // reverse order. The storage stays in place, so it can be constructed again.
    alignas(counted_destructor) hsd::uchar _raw[sizeof(counted_destructor) * 3]{};
    int destroyed = 0;

    auto* _single = hsd::construct_at(reinterpret_cast<counted_destructor*>(_raw), &destroyed);
    hsd::destroy_at(_single);
    assert(destroyed == 1);

    auto* _items = reinterpret_cast<counted_destructor*>(_raw);
    for (hsd::usize index = 0; index < 3; ++index)
    {
        hsd::construct_at(_items + index, &destroyed);
    }
    hsd::destroy_at(reinterpret_cast<counted_destructor(*)[3]>(_items));
    assert(destroyed == 4);

    // A type with a trivial destructor has nothing to run.
    int plain = 5;
    hsd::destroy_at(&plain);
    assert(plain == 5);

    // Each request must return storage aligned to the requested boundary,
    // including boundaries above the default allocation alignment.
    constexpr hsd::usize alignments[] = {8, 16, 32, 64, 256, 4096};
    for (hsd::usize alignment : alignments)
    {
        auto* ptr = static_cast<hsd::uchar*>(
            hsd::new_allocate(100, alignment)
        );
        assert(ptr != nullptr);
        assert(reinterpret_cast<hsd::usize>(ptr) % alignment == 0);

        for (hsd::usize index = 0; index < 100; ++index)
        {
            ptr[index] = static_cast<hsd::uchar>(index);
        }

        hsd::delete_deallocate(ptr, 100, alignment);
    }

    return 0;
}
