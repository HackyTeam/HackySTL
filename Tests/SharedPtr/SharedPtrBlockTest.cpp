#include <Wrappers/Functional.hpp>
#include <Wrappers/SharedPtr.hpp>
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <string>

static long live = 0;

struct Obj
{
    int v;
    std::string pad;

    explicit Obj(int x)
        : v(x), pad(40, 'x')
    {
        ++live;
    }

    Obj(const Obj& o)
        : v(o.v), pad(o.pad)
    {
        ++live;
    }

    ~Obj()
    {
        --live;
    }
};

// Allocator that checks the (pointer, count) contract between allocate and deallocate.
struct tracking_allocator
{
    static inline std::map<void*, hsd::usize> blocks;
    static inline long allocs = 0;
    static inline long frees = 0;

    template <typename T>
    static auto allocate(hsd::usize n)
    {
        auto r = hsd::allocator::allocate<T>(n);

        if (n != 0)
        {
            ++allocs;
            blocks[(void*)r.unwrap()] = n * sizeof(T);
        }

        return r;
    }

    template <typename T>
    static auto deallocate(T* p, hsd::usize n)
    {
        if (p != nullptr)
        {
            auto it = blocks.find((void*)p);
            assert(it != blocks.end());
            assert(it->second == n * sizeof(T));   // same length handed back
            blocks.erase(it);
            ++frees;
        }

        return hsd::allocator::deallocate(p, n);
    }

    friend constexpr bool operator==(tracking_allocator, tracking_allocator) = default;
};

static void reset_alloc()
{
    assert(tracking_allocator::blocks.empty());
    tracking_allocator::allocs = 0;
    tracking_allocator::frees = 0;
}

struct A
{
    virtual ~A()
    {
        ++dtor_a;
    }

    int a = 1;
    static inline int dtor_a = 0;
};

struct B
{
    virtual ~B()
    {
        ++dtor_b;
    }

    int b = 2;
    static inline int dtor_b = 0;
};

struct D : A, B
{
    ~D() override
    {
        ++dtor_d;
    }

    int d = 3;
    static inline int dtor_d = 0;
};

struct alignas(64) Big
{
    int x;

    explicit Big(int v)
        : x(v)
    {
        ++live;
    }

    ~Big()
    {
        --live;
    }
};

struct alignas(16) Mid
{
    int x;

    explicit Mid(int v)
        : x(v)
    {
        ++live;
    }

    ~Mid()
    {
        --live;
    }
};

// The operations the randomized comparison picks from.
enum class action
{
    make,
    copy_assign,
    reset,
    count_check,
    move_assign,
    swap,
    temporary_copies
};

template <template <typename...> class Ptr, typename Maker>
static void run_suite(const char* name, Maker make)
{
    // Single allocation, with matching sizes handed back.
    {
        reset_alloc();

        {
            auto p = make.template operator()<Obj, tracking_allocator>(7);
            assert(tracking_allocator::allocs == 1);
            assert(p->v == 7 && live == 1 && p.get_count() == 1 && p.is_unique());

            auto q = p;
            assert(p.get_count() == 2 && q.get_count() == 2 && q.get() == p.get());

            q = nullptr;
            assert(p.get_count() == 1 && q.get_count() == 0);
        }

        assert(live == 0 && tracking_allocator::frees == 1);
        reset_alloc();
    }

    // Caller-supplied allocator object.
    {
        tracking_allocator alloc;

        {
            auto p = make.template operator()<Obj, tracking_allocator>(alloc, 11);
            assert(p->v == 11 && tracking_allocator::allocs == 1);
        }

        assert(live == 0 && tracking_allocator::frees == 1);
        reset_alloc();
    }

    // Over-aligned types keep the old two-block layout and still work.
    {
        {
            auto p = make.template operator()<Big, tracking_allocator>(3);
            assert(tracking_allocator::allocs == 2);
            assert(reinterpret_cast<std::uintptr_t>(p.get()) % 64 == 0);

            auto q = p;
            assert(q->x == 3 && p.get_count() == 2);
        }

        assert(live == 0 && tracking_allocator::frees == 2);
        reset_alloc();

        {
            auto p = make.template operator()<Mid, tracking_allocator>(4);
            assert(reinterpret_cast<std::uintptr_t>(p.get()) % 16 == 0 && p->x == 4);
        }

        assert(live == 0);
        reset_alloc();
    }

    // Derived to base, including a base that sits at a non-zero offset.
    {
        A::dtor_a = B::dtor_b = D::dtor_d = 0;

        {
            auto d = make.template operator()<D, tracking_allocator>();
            Ptr<B, tracking_allocator> b = d;
            Ptr<A, tracking_allocator> a = d;
            assert(b->b == 2 && a->a == 1 && d.get_count() == 3);

            d = nullptr;
            a = nullptr;
            assert(b.get_count() == 1 && D::dtor_d == 0);
        }

        assert(D::dtor_d == 1 && B::dtor_b == 1 && A::dtor_a == 1);
        assert(tracking_allocator::blocks.empty());
        reset_alloc();
    }

    // Empty owners.
    {
        Ptr<Obj, tracking_allocator> e1;
        Ptr<Obj, tracking_allocator> e2{nullptr};
        assert(e1.get() == nullptr && e1.get_count() == 0 && e1 == nullptr && e2 == nullptr);
        assert(tracking_allocator::allocs == 0);   // no counter for an empty owner

        Ptr<Obj, tracking_allocator> e3 = e1;      // copy of empty
        e1 = e2;                                   // empty = empty
        assert(e3.get() == nullptr && e3.get_count() == 0);

        auto p = make.template operator()<Obj, tracking_allocator>(1);
        e1 = p;                                    // empty = full
        assert(p.get_count() == 2 && e1.get() == p.get());

        p = e2;                                    // full = empty
        assert(p.get() == nullptr && e1.get_count() == 1);

        e1 = nullptr;
        Ptr<Obj, tracking_allocator> e4 = e1;      // copy after release
        assert(e4.get() == nullptr && live == 0);

        e1 = nullptr;
        e1 = nullptr;                              // releasing twice is fine
    }

    assert(tracking_allocator::blocks.empty());
    reset_alloc();

    // Move and swap.
    {
        auto p = make.template operator()<Obj, tracking_allocator>(5);
        Ptr<Obj, tracking_allocator> q;
        q = hsd::move(p);

        assert(q->v == 5 && q.get_count() == 1);

        auto r = make.template operator()<Obj, tracking_allocator>(6);
        hsd::swap(q, r);

        assert(q->v == 6 && r->v == 5 && live == 2);
        assert(q.get_count() == 1 && r.get_count() == 1);
    }

    assert(live == 0);
    reset_alloc();

    // Randomized comparison with std::shared_ptr.
    {
        std::mt19937 rng(2024);
        constexpr int N = 12;
        Ptr<Obj, tracking_allocator> h[N];
        std::shared_ptr<Obj> s[N];

        for (int step = 0; step < 200000; ++step)
        {
            int i = rng() % N;
            int j = rng() % N;
            auto op = static_cast<action>(rng() % 7);

            switch (op)
            {
                case action::make:
                {
                    int v = rng() % 1000;
                    h[i] = make.template operator()<Obj, tracking_allocator>(int{v});
                    s[i] = std::make_shared<Obj>(v);
                    break;
                }
                case action::copy_assign:
                {
                    h[i] = h[j];
                    s[i] = s[j];
                    break;
                }
                case action::reset:
                {
                    h[i] = nullptr;
                    s[i] = nullptr;
                    break;
                }
                case action::count_check:
                {
                    auto c = h[j];
                    auto d = s[j];
                    assert(c.get_count() == (hsd::usize)d.use_count());
                    break;
                }
                case action::move_assign:
                {
                    h[i] = hsd::move(h[j]);
                    s[i] = std::move(s[j]);

                    if (i == j)
                    {
                        s[i] = nullptr;
                        h[i] = nullptr;
                    }

                    break;
                }
                case action::swap:
                {
                    hsd::swap(h[i], h[j]);
                    std::swap(s[i], s[j]);
                    break;
                }
                case action::temporary_copies:
                {
                    Ptr<Obj, tracking_allocator> c{h[i]};
                    std::shared_ptr<Obj> d{s[i]};
                    (void)c;
                    (void)d;
                    break;
                }
            }

            for (int k = 0; k < N; ++k)
            {
                assert((h[k].get() == nullptr) == (s[k] == nullptr));

                if (s[k])
                {
                    assert(h[k]->v == s[k]->v);
                    assert(h[k].get_count() == (hsd::usize)s[k].use_count());
                }
            }
        }

        for (int k = 0; k < N; ++k)
        {
            h[k] = nullptr;
            s[k] = nullptr;
        }
    }

    assert(live == 0 && tracking_allocator::blocks.empty());
    reset_alloc();
    (void)name;
}

struct atomic_maker
{
    template <typename T, typename Alloc, typename... Args>
    auto operator()(Args&&... a) const
    {
        return hsd::make_safe_shared<T, Alloc>(hsd::forward<Args>(a)...);
    }
};

struct plain_maker
{
    template <typename T, typename Alloc, typename... Args>
    auto operator()(Args&&... a) const
    {
        return hsd::make_unsafe_shared<T, Alloc>(hsd::forward<Args>(a)...);
    }
};

template <typename T, typename Alloc>
using atomic_ptr = hsd::safe_shared_ptr<T, Alloc>;

template <typename T, typename Alloc>
using plain_ptr = hsd::unsafe_shared_ptr<T, Alloc>;

int main()
{
    run_suite<atomic_ptr>("atomic", atomic_maker{});
    run_suite<plain_ptr>("non-atomic", plain_maker{});

    // Arrays and adopted raw pointers still use the old layout.
    {
        tracking_allocator alloc;

        {
            auto arr = hsd::make_safe_shared<int[], tracking_allocator>(alloc, hsd::usize{10});
            assert(tracking_allocator::allocs == 2);

            for (int i = 0; i < 10; ++i)
            {
                arr.get()[i] = i;
            }

            auto c = arr;
            assert(c.get()[9] == 9 && arr.get_count() == 2);
        }

        assert(tracking_allocator::blocks.empty());
        reset_alloc();

        {
            auto* raw = hsd::allocator::allocate<Obj>(1).unwrap();
            hsd::construct_at(raw, 9);
            hsd::safe_shared_ptr<Obj> p{raw};
            auto q = p;
            assert(p.get_count() == 2 && q->v == 9);
        }

        assert(live == 0);
    }

    // hsd::function: small buffer, heap-held, copy, move, swap, empty.
    {
        int k = 3;
        hsd::function<int(int)> small = [k](int x) { return x + k; };
        long big_live_before = live;

        {
            auto tracked = std::make_shared<Obj>(1);
            (void)tracked;

            Obj o(41);
            char pad[96] = {};

            hsd::function<int(int)> big = [o, pad](int x) { return x + o.v + (int)pad[0]; };
            auto big2 = big;

            hsd::function<int(int)> big3;
            big3 = big2;

            hsd::function<int(int)> empty;

            assert(big(1).unwrap() == 42 && big2(1).unwrap() == 42 && big3(1).unwrap() == 42);
            assert(small(1).unwrap() == 4);
            assert(!static_cast<bool>(empty) && !empty(1).is_ok());

            // Swap exchanges the two targets, even when they use different storage.
            hsd::swap(small, big3);
            assert(small(1).unwrap() == 42 && big3(1).unwrap() == 4);

            // Swap back, so `small` does not keep the heap target alive past this block.
            hsd::swap(small, big3);
            assert(small(1).unwrap() == 4 && big3(1).unwrap() == 42);

            // The moved-to wrapper keeps the target.
            auto moved = hsd::move(big2);
            assert(moved(3).unwrap() == 44);

            big3 = nullptr;
            assert(!static_cast<bool>(big3));
        }

        assert(live == big_live_before);
    }

    assert(live == 0);
    return 0;
}
