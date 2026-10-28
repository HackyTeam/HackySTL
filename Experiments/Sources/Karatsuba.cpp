#include <Serialize/Io.hpp>
#include <Containers/Integer.hpp>

int main()
{
    using namespace hsd::format_literals;

    hsd::integer<128> _lhs = 0xFFFF'FFFF'FFFF'FFFFULL;
    hsd::integer<128> _rhs = 0xFFFF'FFFF'FFFF'FFFFULL;

    hsd::u128 a = 0xFFFF'FFFF'FFFF'FFFFULL;

    hsd::println("{}"_fmt, _lhs * _rhs);
    hsd::println("{}"_fmt, a * a);
}