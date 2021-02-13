#include <UniquePtr.hpp>

struct base
{
    int a, b, c, d;
    virtual void overload() = 0;
    virtual ~base() {}
};

struct derived
    : public base
{
    int e, f, g, h;
    virtual void overload() override
    {}
};

template <typename T>
struct hsdCTAlloc
    : public hsd::constexpr_allocator<T, 1>
{};

int main()
{
    hsd::uchar buf[128]{};
    hsd::buffered_allocator<int> alloc{buf, 128};

    auto a = hsd::make_unique<int>(1112);
    hsd::unique_ptr<int> b = hsd::move(a);
    hsd::unique_ptr<int[], hsd::buffered_allocator> c = 
        hsd::make_unique<int[], hsd::buffered_allocator>(alloc, 5);
    hsd::unique_ptr<base, hsd::buffered_allocator> d = 
        hsd::make_unique<derived, hsd::buffered_allocator>(alloc);
}