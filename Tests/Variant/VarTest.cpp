#include <Variant.hpp>
#include <Io.hpp>

using namespace hsd::format_literals;

template <typename T>
struct verbose 
{
    verbose() 
    {
        hsd::println("{} default constructor"_fmt, T::name());
    }

    ~verbose() 
    {
        hsd::println("{} destructor"_fmt, T::name());
    }

    verbose(verbose const&) 
    {
        hsd::println("{} copy constructor"_fmt, T::name());
    }

    verbose(verbose&&) 
    {
        hsd::println("{} move constructor"_fmt, T::name());
    }

    auto& operator=(verbose const&) 
    {
        hsd::println("{} copy assignment"_fmt, T::name());
        return *this;
    }

    auto& operator=(verbose&&) 
    {
        hsd::println("{} move assignment"_fmt, T::name());
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
        hsd::println("B constructor"_fmt);
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
    
    hsd::println("{}"_fmt, hsd::variant_size<decltype(va4)>::value);

    hsd::println("{}"_fmt, va1.index());
    hsd::println("{}"_fmt, va2.index());
    hsd::println("{}"_fmt, va3.index());
    hsd::println("{}"_fmt, va4.index());

    hsd::println("{}"_fmt, va1.get<0>().unwrap().get());
    
    auto _res = va1.get<hsd::f32>();

    if(_res) 
    {
        hsd::println("{}"_fmt, _res.unwrap().get());
    } 
    else
    {
        hsd::println_err("Caught error: {}"_fmt, _res.unwrap_err().pretty_error());
    }

    if (auto* val = hsd::get_if<hsd::f32>(&va1)) 
    {
        hsd::println("Got value: {}"_fmt, *val);
    } 
    else 
    {
        hsd::println("No value of type float in va1"_fmt);
    }

    if (auto* val = va2.get_if<B>()) 
    {
        hsd::println("B inside va2: {}"_fmt, val->_a);
    } 
    else 
    {
        hsd::println("No B in va2"_fmt);
    }

    hsd::println("va2 == va3: {}"_fmt, (va2 == va3) ? "true" : "false");
    va2 = va3;
    hsd::println("(after assignment) {}"_fmt, (va2 == va3) ? "true" : "false");
    va3 = hsd::variant<A, B>(6);
    hsd::println("(after new assignment)\nva3 == va4: {}"_fmt, (va3 == va4) ? "true" : "false");
    va3.get<B>().unwrap().get()._a = 5;
    hsd::println("(after modification) {}\n"_fmt, (va3 == va4) ? "true" : "false");

    hsd::variant<A, B> va5;

    va5.visit([](auto&&) 
    {
        hsd::println("Auto deduction: {}"_fmt, HSD_FUNCTION_NAME + 28);
    });

    va5 = B(42);

    va5.visit([](auto&&) 
    {
        hsd::println("Auto deduction: {}"_fmt, HSD_FUNCTION_NAME + 28);
    });

    hsd::variant<verbose<void>, verbose<void>> va6;
    auto va7 = va6;
}
