#include <Base/Utility.hpp>
#include <cassert>

constexpr int construct_in_constant_evaluation()
{
    hsd::u32 storage[2]{};
    hsd::construct_at(&storage[1], 42u);
    return static_cast<int>(storage[1]);
}

static_assert(construct_in_constant_evaluation() == 42);

int main()
{
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
