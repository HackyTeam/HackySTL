#include <Wrappers/Functional.hpp>
#include <cassert>

struct tracked
{
    static inline int live = 0;
    int v;

    tracked(int x)
        : v(x)
    {
        ++live;
    }

    tracked(const tracked& o)
        : v(o.v)
    {
        ++live;
    }

    tracked(tracked&& o) noexcept
        : v(o.v)
    {
        ++live;
    }

    ~tracked()
    {
        --live;
    }
};

// Assigning null and then destroying the wrapper must destroy the target once.
static void test_null_assign_then_destroy()
{
    {
        hsd::function<int(int)> f = [t = tracked(1)](int x) { return x + t.v; };
        f = nullptr;
        assert(!static_cast<bool>(f));
    }

    assert(tracked::live == 0);
}

// Move-assigning releases the destination's previous target.
static void test_move_assign_releases_target()
{
    {
        hsd::function<int(int)> f1 = [t = tracked(2)](int x) { return x + t.v; };
        hsd::function<int(int)> f2 = [t = tracked(3)](int x) { return x + t.v; };

        f1 = hsd::move(f2);
        assert(f1(0).unwrap() == 3);

        // f1 holds one target, and f2 still holds its moved-from target.
        assert(tracked::live == 2);
    }

    assert(tracked::live == 0);
}

// A small target moved into a wrapper that holds a heap target must replace it.
static void test_small_into_heap()
{
    int k = 3;
    hsd::function<int(int)> small = [k](int x) { return x + k; };

    char pad[96] = {};
    hsd::function<int(int)> big = [pad](int x) { return x + 41 + (int)pad[0]; };

    hsd::function<int(int)> tmp = hsd::move(small);
    big = hsd::move(tmp);
    assert(big(1).unwrap() == 4);
}

// Swapping exchanges the targets, even when one uses inline storage and the other the heap.
static void test_swap_mixed_storage()
{
    int k = 3;
    hsd::function<int(int)> small = [k](int x) { return x + k; };

    char pad[96] = {};
    hsd::function<int(int)> big = [pad](int x) { return x + 41 + (int)pad[0]; };

    hsd::swap(small, big);
    assert(small(1).unwrap() == 42);
    assert(big(1).unwrap() == 4);
}

// Self-assignment keeps the target.
static void test_self_assign()
{
    int k = 5;
    hsd::function<int(int)> f = [k](int x) { return x + k; };

    auto& same = f;
    f = same;
    assert(f(1).unwrap() == 6);

    f = hsd::move(f);
    assert(f(1).unwrap() == 6);
}

int main()
{
    test_null_assign_then_destroy();
    test_move_assign_releases_target();
    test_small_into_heap();
    test_swap_mixed_storage();
    test_self_assign();
    return 0;
}
