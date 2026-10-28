// Title: Limits table
// Level: Basic
// Shows: min, max, is_signed and digits from hsd::limits for the integer and floating point types.

#include <Base/Limits.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// limits<T> exposes its values as static constants, so one template prints any integer type.
// The 8 bit types are character types, so they are widened to print as numbers.
template <typename T>
void print_integer_row(const char* name)
{
    hsd::println("{}: min={} max={} signed={} digits={}"_fmt,
        name, static_cast<hsd::i64>(hsd::limits<T>::min),
        static_cast<hsd::u64>(hsd::limits<T>::max),
        hsd::limits<T>::is_signed ? "yes" : "no", hsd::limits<T>::digits);
}

int main()
{
    print_integer_row<hsd::i8>("i8");
    print_integer_row<hsd::u8>("u8");
    print_integer_row<hsd::i16>("i16");
    print_integer_row<hsd::u16>("u16");
    print_integer_row<hsd::i32>("i32");
    print_integer_row<hsd::u32>("u32");
    print_integer_row<hsd::i64>("i64");
    print_integer_row<hsd::u64>("u64");

    // For floating point types min is the smallest positive normal value,
    // not the most negative one, and epsilon is the gap above 1.
    hsd::println("f32: max={exp} min={exp} epsilon={exp}"_fmt,
        hsd::limits<hsd::f32>::max, hsd::limits<hsd::f32>::min, hsd::limits<hsd::f32>::epsilon);
    hsd::println("f64: max={exp} min={exp} epsilon={exp}"_fmt,
        hsd::limits<hsd::f64>::max, hsd::limits<hsd::f64>::min, hsd::limits<hsd::f64>::epsilon);
    hsd::println("f32 mantissa bits={} f64 mantissa bits={}"_fmt,
        hsd::limits<hsd::f32>::digits, hsd::limits<hsd::f64>::digits);

    // Infinity is a limit too, and it compares above every finite value.
    hsd::println("infinity above f64 max: {}"_fmt,
        hsd::limits<hsd::f64>::infinity > hsd::limits<hsd::f64>::max ? "true" : "false");

    return 0;
}
