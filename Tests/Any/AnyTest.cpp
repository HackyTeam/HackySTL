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
    // any type
    hsd::any a = 1;
    hsd_println("{}", a.cast_to<hsd::i32>().unwrap());
    a = 3.14;
    hsd_println("{}", a.cast_to<hsd::f64>().unwrap());
    a = true;
    hsd_println("{}", a.cast_to<bool>().unwrap());
 
    // bad cast
    a = 1;
    hsd_println("{}", a.cast_to<hsd::f32>().unwrap_err()());
 
    // has value
    a = 1;
    
    if (a.has_value())
        hsd_println("Has value");
 
    // reset
    a.reset();
    
    if (!a.has_value())
        hsd_println("No value");
 
    // pointer to contained data
    a = 1;
    
    if (int* i = a.cast_if<hsd::i32>())
        hsd_println("{}", *i);

    // holds a given type
    a.emplace<S>(1, 'c', 4.2f, "str");
    
    if (a.holds_type<S>())
        hsd_println("Holds a S");
    
    if (a.holds_type<hsd::i32>())
        hsd_println("Holds an int");
}