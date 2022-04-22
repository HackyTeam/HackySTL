#include <Io.hpp>
#include <SharedPtr.hpp>

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
    hsd::println(
        "{}\n{}\n{}\n{}\n"_fmt, 
        ptr->_a, ptr->_b, 
        ptr->_c, ptr->_d
    );
}

static void print(
    hsd::unsafe_shared_ptr<S, hsd::buffered_allocator> ptr)
{
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
        hsd::buffered_allocator<int> alloc{buf, 128};

        hsd::safe_shared_ptr<S, hsd::buffered_allocator> elm = 
            hsd::make_safe_shared<S>(alloc, 12, 'c', 4.3f, "str");

        hsd::safe_shared_ptr<S, hsd::buffered_allocator> elm2 = elm;
        print(elm);
        print(elm2);
        elm2 = hsd::move(elm);

        hsd::safe_shared_ptr<base> elm3 = 
            hsd::make_safe_shared<derived>();

        hsd::safe_shared_ptr<base> elm4;
        elm4 = elm3;
        elm4 = hsd::move(elm3);
    }

    {
        hsd::println("Unsafe version:"_fmt);
        
        hsd::uchar buf[128]{};
        hsd::buffered_allocator<int> alloc{buf, 128};

        hsd::unsafe_shared_ptr<S, hsd::buffered_allocator> elm = 
            hsd::make_unsafe_shared<S>(alloc, 12, 'c', 4.3f, "str");

        hsd::unsafe_shared_ptr<S, hsd::buffered_allocator> elm2 = elm;
        print(elm);
        print(elm2);
        elm2 = hsd::move(elm);

        hsd::unsafe_shared_ptr<base> elm3 = 
            hsd::make_unsafe_shared<derived>();

        hsd::unsafe_shared_ptr<base> elm4;
        elm4 = elm3;
        elm4 = hsd::move(elm3);
    }
}