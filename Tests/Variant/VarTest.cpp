#include <Variant.hpp>
#include <Io.hpp>

template <typename T>
struct verbose 
{
    verbose() 
    {
        hsd_println("{} default constructor", T::name());
    }

    ~verbose() 
    {
        hsd_println("{} destructor", T::name());
    }

    verbose(verbose const&) 
    {
        hsd_println("{} copy constructor", T::name());
    }

    verbose(verbose&&) 
    {
        hsd_println("{} move constructor", T::name());
    }

    auto& operator=(verbose const&) 
    {
        hsd_println("{} copy assignment", T::name());
        return *this;
    }

    auto& operator=(verbose&&) 
    {
        hsd_println("{} move assignment", T::name());
        return *this;
    }
    
protected:
    verbose(int) {} // Silent constructor
};

template <>
struct verbose<void> 
    : verbose<verbose<void>> 
{
    constexpr static char const* name() 
    {
        return "verbose"; 
    }
};

struct A : verbose<A> 
{
    constexpr static char const* name() 
    {
        return "A"; 
    }
};
struct B : verbose<B> 
{
    constexpr static char const* name() 
    {
        return "B"; 
    }
    
    int _a;
    
    B(int a) 
        : verbose<B>(0), _a(a) 
    {
        hsd_println("B constructor");
    }
};

constexpr bool operator==(A const&, A const&) noexcept 
{
    return true; 
}

constexpr bool operator==(B const& a, B const& b) noexcept 
{
    return a._a == b._a;
}

int main() 
{
    hsd::variant<hsd::i32, hsd::f32> va1(5);
    hsd::variant<A, B> va2(5);
    hsd::variant<A, B> va3{A()};
    auto va4(va2);
    
    hsd_println("{}", hsd::variant_size<decltype(va4)>::value);

    hsd_println("{}", va1.index());
    hsd_println("{}", va2.index());
    hsd_println("{}", va3.index());
    hsd_println("{}", va4.index());

    hsd_println("{}", va1.get<0>().unwrap());
    
    auto _res = va1.get<hsd::f32>();

    if(_res) 
    {
        // This actually throws 
        hsd_println("{}", _res.unwrap());
    } 
    else
    {
        hsd_println_err("Caught error: {}", _res.unwrap_err()());
    }

    if (auto* val = hsd::get_if<hsd::f32>(&va1)) 
    {
        hsd_println("Got value: {}", *val);
    } 
    else 
    {
        hsd_println("No value of type float in va1");
    }
    if (auto* val = va2.get_if<B>()) 
    {
        hsd_println("B inside va2: {}", val->_a);
    } 
    else 
    {
        hsd_println("No B in va2");
    }

    hsd_println("va2 == va3: {}", (va2 == va3) ? "true" : "false");
    va2 = va3;
    hsd_println("(after assignment) {}", (va2 == va3) ? "true" : "false");
    va3 = hsd::variant<A, B>(6);
    hsd_println("(after new assignment)\nva3 == va4: {}", (va3 == va4) ? "true" : "false");
    va3.get<B>().unwrap()._a = 5;
    hsd_println("(after modification) {}\n", (va3 == va4) ? "true" : "false");

    hsd::variant<A, B> va5;

    va5.visit([](auto&&) 
    {
        hsd_println("Auto deduction: {}", HSD_FUNCTION_NAME + 28);
    });

    va5 = B(42);

    va5.visit([](auto&&) 
    {
        hsd_println("Auto deduction: {}", HSD_FUNCTION_NAME + 28);
    });

    hsd::variant<verbose<void>, verbose<void>> va6;
    auto va7 = va6;
}
