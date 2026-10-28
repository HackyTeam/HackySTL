#include <Serialize/Io.hpp>
#include <Wrappers/SharedPtr.hpp>
#include <assert.h>
#include <memory>

struct counted_value
{
    static inline int alive = 0;

    counted_value()
    {
        ++alive;
    }

    ~counted_value()
    {
        --alive;
    }
};

struct S
{
    int _a;
    char _b;
    float _c;
    const char* _d;

    constexpr S(int a, char b, float c, const char* d)
        : _a{a}, _b{b}, _c{c}, _d{d}
    {}
};

using namespace hsd::format_literals;

static void print(
    hsd::safe_shared_ptr<S, hsd::buffered_allocator> ptr)
{
    assert(ptr.get() != nullptr);
    assert(ptr->_a == 12 && ptr->_b == 'c');
    hsd::println(
        "{}\n{}\n{}\n{}\n"_fmt, 
        ptr->_a, ptr->_b, 
        ptr->_c, ptr->_d
    );
}

static void print(
    hsd::unsafe_shared_ptr<S, hsd::buffered_allocator> ptr)
{
    assert(ptr.get() != nullptr);
    assert(ptr->_a == 12 && ptr->_b == 'c');
    hsd::println(
        "{}\n{}\n{}\n{}"_fmt, 
        ptr->_a, ptr->_b, 
        ptr->_c, ptr->_d
    );
}

struct base
{
    virtual void overload() = 0;
    virtual ~base() {}
};

struct derived
    : public base
{
    virtual void overload() override
    {}
};

int main()
{
    {
        hsd::println("Safe version:"_fmt);
        
        hsd::uchar buf[128]{};
        hsd::buffered_allocator alloc{buf, 128};

        auto elm = hsd::make_safe_shared<S, hsd::buffered_allocator>(
            alloc, 12, 'c', 4.3f, "str"
        );
        auto standard_elm = std::make_shared<S>(12, 'c', 4.3f, "str");

        hsd::safe_shared_ptr<S, hsd::buffered_allocator> elm2 = elm;
        auto standard_elm2 = standard_elm;
        assert(elm.get_count() == 2);
        assert(elm.get_count() == static_cast<hsd::usize>(standard_elm.use_count()));
        assert(elm == elm2);
        assert(standard_elm == standard_elm2);
        assert(!(elm != elm2));
        assert(elm != nullptr);
        assert(standard_elm != nullptr);
        assert(!(elm == nullptr));
        print(elm);
        print(elm2);
        elm2 = hsd::move(elm);
        standard_elm2 = std::move(standard_elm);
        assert(elm2.get_count() == 1);
        assert(elm2.get_count() == static_cast<hsd::usize>(standard_elm2.use_count()));
        assert(standard_elm == nullptr);

        hsd::safe_shared_ptr<base> elm3 = 
            hsd::make_safe_shared<derived>();

        hsd::safe_shared_ptr<base> elm4;
        assert(elm4 == nullptr);
        assert(!(elm4 != nullptr));
        elm4 = elm3;
        assert(elm4 != nullptr);
        elm4 = hsd::move(elm3);
        assert(elm4.get() != nullptr);
    }

    {
        hsd::println("Unsafe version:"_fmt);
        
        hsd::uchar buf[128]{};
        hsd::buffered_allocator alloc{buf, 128};

        auto elm = hsd::make_unsafe_shared<S, hsd::buffered_allocator>(
            alloc, 12, 'c', 4.3f, "str"
        );
        auto standard_elm = std::make_shared<S>(12, 'c', 4.3f, "str");

        hsd::unsafe_shared_ptr<S, hsd::buffered_allocator> elm2 = elm;
        auto standard_elm2 = standard_elm;
        assert(elm.get_count() == 2);
        assert(elm.get_count() == static_cast<hsd::usize>(standard_elm.use_count()));
        assert(elm == elm2);
        assert(standard_elm == standard_elm2);
        assert(!(elm != elm2));
        assert(elm != nullptr);
        assert(!(elm == nullptr));
        print(elm);
        print(elm2);
        elm2 = hsd::move(elm);
        standard_elm2 = std::move(standard_elm);
        assert(elm2.get_count() == 1);
        assert(elm2.get_count() == static_cast<hsd::usize>(standard_elm2.use_count()));
        assert(standard_elm == nullptr);

        hsd::unsafe_shared_ptr<base> elm3 = 
            hsd::make_unsafe_shared<derived>();

        hsd::unsafe_shared_ptr<base> elm4;
        assert(elm4 == nullptr);
        assert(!(elm4 != nullptr));
        elm4 = elm3;
        assert(elm4 != nullptr);
        elm4 = hsd::move(elm3);
        assert(elm4.get() != nullptr);
    }

    {
        auto values = hsd::make_unsafe_shared<counted_value[]>(4);
        auto copy = values;
        assert(counted_value::alive == 4);
    }

    assert(counted_value::alive == 0);

    {
        auto values = hsd::make_safe_shared<counted_value[]>(3);
        auto copy = values;
        assert(counted_value::alive == 3);
    }

    assert(counted_value::alive == 0);
}