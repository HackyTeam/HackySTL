#include <Wrappers/Any.hpp>
#include <cassert>

// `counted<1>` fits the inline buffer and `counted<64>` lives on the heap.
template <hsd::usize Pad>
struct counted
{
    static inline int live = 0;
    int value;
    unsigned char pad[Pad];

    counted(int v)
        : value{v}, pad{}
    {
        ++live;
    }

    counted(const counted& other)
        : value{other.value}, pad{}
    {
        ++live;
    }

    counted(counted&& other) noexcept
        : value{other.value}, pad{}
    {
        ++live;
    }

    ~counted()
    {
        --live;
    }
};

struct Large
{
    long values[12];
};

static void test_inline_value()
{
    {
        hsd::any value{counted<1>{7}};
        assert(counted<1>::live == 1);

        hsd::any copy{value};
        assert(counted<1>::live == 2);

        // Moving an inline value destroys the source's copy.
        hsd::any moved{hsd::move(copy)};
        assert(!copy.has_value());
        assert(moved.holds_type<counted<1>>());
        assert(counted<1>::live == 2);
    }

    assert(counted<1>::live == 0);
}

static void test_heap_value()
{
    {
        hsd::any big{counted<64>{3}};
        const counted<64>* before = big.cast_if<counted<64>>();

        // Moving a heap value keeps its block, so the address stays the same.
        hsd::any taken{hsd::move(big)};
        assert(taken.cast_if<counted<64>>() == before);
        assert(!big.has_value());

        // A copy of a heap value is an independent object.
        hsd::any copy{taken};
        assert(copy.cast_if<counted<64>>() != taken.cast_if<counted<64>>());
        assert(counted<64>::live == 2);
    }

    assert(counted<64>::live == 0);
}

static void test_swap_inline_and_heap()
{
    hsd::any small{1};
    hsd::any large{Large{}};

    hsd::swap(small, large);
    assert(small.holds_type<Large>() && large.holds_type<int>());

    // Swapping a value with itself keeps it.
    small.swap(small);
    assert(small.holds_type<Large>());
}

static void test_emplace_from_old_value()
{
    hsd::any value{5};
    value.emplace<int>(*value.cast_if<int>() + 1);
    assert(*value.cast_if<int>() == 6);

    // Emplacing replaces a heap value and destroys it exactly once.
    hsd::any held{counted<64>{1}};
    held.emplace<counted<1>>(2);
    assert(counted<64>::live == 0 && counted<1>::live == 1);

    held.reset();
    assert(!held.has_value() && counted<1>::live == 0);
}

int main()
{
    test_inline_value();
    test_heap_value();
    test_swap_inline_and_heap();
    test_emplace_from_old_value();
    return 0;
}
