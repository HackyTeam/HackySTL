#include <Limits.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::format_literals;

    hsd::println("Upper limits:"_fmt);

    hsd::println("the limit is: {}"_fmt, hsd::limits<hsd::i16>::max);
    hsd::println("the limit is: {}"_fmt, hsd::limits<hsd::i32>::max);
    hsd::println("the limit is: {}"_fmt, hsd::limits<hsd::i64>::max);
    hsd::println("the limit is: {exp}"_fmt, hsd::limits<hsd::f32>::max);
    hsd::println("the limit is: {exp}"_fmt, hsd::limits<hsd::f64>::max);
    hsd::println("the limit is: {exp}"_fmt, hsd::limits<hsd::f128>::max);

    hsd::println("Lower limits:"_fmt);

    hsd::println("the limit is: {}"_fmt, hsd::limits<hsd::i16>::min);
    hsd::println("the limit is: {}"_fmt, hsd::limits<hsd::i32>::min);
    hsd::println("the limit is: {}"_fmt, hsd::limits<hsd::i64>::min);
    hsd::println("the limit is: {exp}"_fmt, hsd::limits<hsd::f32>::min);
    hsd::println("the limit is: {exp}"_fmt, hsd::limits<hsd::f64>::min);
    hsd::println("the limit is: {exp}"_fmt, hsd::limits<hsd::f128>::min);
}