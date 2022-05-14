#include <Io.hpp>
#include <Any.hpp>

struct S
{
    hsd::i32 _a;
    char _b;
    hsd::f32 _c;
    const char* _d;

    S(hsd::i32 a, char b, hsd::f32 c, const char* d) : _a(a), _b(b), _c(c), _d(d) {}
};

int main()
{
    using namespace hsd::format_literals;

    // any type
    hsd::any a = 1;
    hsd::println("{}"_fmt, a.cast_to<hsd::i32>().unwrap().get());
    a = 3.14;
    hsd::println("{}"_fmt, a.cast_to<hsd::f64>().unwrap().get());
    a = true;
    hsd::println("{}"_fmt, a.cast_to<bool>().unwrap().get() ? "true" : "false");
 
    // bad cast
    a = 1;
    hsd::println("{}"_fmt, a.cast_to<hsd::f32>().unwrap_err().pretty_error());
 
    // has value
    a = 1;
    
    if (a.has_value())
    {
        hsd::println("Has value"_fmt);
    }
 
    // reset
    a.reset();
    
    if (!a.has_value())
    {
        hsd::println("No value"_fmt);
    }
 
    // pointer to contained data
    a = 1;
    
    if (int* i = a.cast_if<hsd::i32>())
    {
        hsd::println("{}"_fmt, *i);
    }

    // holds a given type
    a.emplace<S>(1, 'c', 4.2f, "str");
    
    if (a.holds_type<S>())
    {
        hsd::println("Holds a S"_fmt);
    }
    
    if (a.holds_type<hsd::i32>())
    {
        hsd::println("Holds an int"_fmt);
    }
}