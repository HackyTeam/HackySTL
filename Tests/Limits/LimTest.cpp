#include <Limits.hpp>
#include <Io.hpp>

int main()
{
    hsd_println("Upper limits:");

    hsd_println("the limit is: {}", hsd::limits<hsd::i16>::max);
    hsd_println("the limit is: {}", hsd::limits<hsd::i32>::max);
    hsd_println("the limit is: {}", hsd::limits<hsd::i64>::max);
    hsd_println("the limit is: {}", hsd::limits<hsd::f32>::max);
    hsd_println("the limit is: {}", hsd::limits<hsd::f64>::max);
    hsd_println("the limit is: {}", hsd::limits<hsd::f128>::max);

    hsd_println("Lower limits: {}", static_cast<hsd::i32>(hsd::limits<hsd::f32>::infinity));

    hsd_println("the limit is: {}", hsd::limits<hsd::i16>::min);
    hsd_println("the limit is: {}", hsd::limits<hsd::i32>::min);
    hsd_println("the limit is: {}", hsd::limits<hsd::i64>::min);
    hsd_println("the limit is: {}", hsd::limits<hsd::f32>::min);
    hsd_println("the limit is: {}", hsd::limits<hsd::f64>::min);
    hsd_println("the limit is: {}", hsd::limits<hsd::f128>::min);
}