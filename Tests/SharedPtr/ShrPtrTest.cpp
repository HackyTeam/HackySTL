#include <iostream>
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

template<typename T>
static void print_val(T& val)
{
    std::cout << val << '\n';
}

static void print(
    hsd::safe_shared_ptr<S, hsd::buffered_allocator> ptr)
{
    print_val(ptr->_a);
    print_val(ptr->_b);
    print_val(ptr->_c);
    print_val(ptr->_d);
}

static void print(
    hsd::unsafe_shared_ptr<S, hsd::buffered_allocator> ptr)
{
    print_val(ptr->_a);
    print_val(ptr->_b);
    print_val(ptr->_c);
    print_val(ptr->_d);
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
        puts("Safe version:");
        hsd::uchar buf[128]{};
        hsd::buffered_allocator<int> alloc{buf, 128};

        hsd::safe_shared_ptr elm = 
            hsd::make_safe_shared<S>(alloc, 12, 'c', 4.3f, "str");

        hsd::safe_shared_ptr elm2 = elm;
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
        puts("\nUnsafe version:");
        hsd::uchar buf[128]{};
        hsd::buffered_allocator<int> alloc{buf, 128};

        hsd::unsafe_shared_ptr elm = 
            hsd::make_unsafe_shared<S>(alloc, 12, 'c', 4.3f, "str");

        hsd::unsafe_shared_ptr elm2 = elm;
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