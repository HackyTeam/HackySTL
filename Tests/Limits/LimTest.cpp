#include "../../cpp/Limits.hpp"
#include "../../cpp/Io.hpp"

int main()
{
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::i32>::max);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::u32>::max);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::u64>::max);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::f32>::max);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::f64>::max);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::f128>::max);

    hsd::io::print<"\n">();

    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::i16>::min);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::i32>::min);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::i64>::min);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::f32>::min);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::f64>::min);
    hsd::io::print<"the limit is: {}\n">(hsd::limits<hsd::f128>::min);
}