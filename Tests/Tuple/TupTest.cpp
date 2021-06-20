#include <Io.hpp>
#include <Tuple.hpp>

int main()
{
    setlocale(LC_ALL, "en_US.UTF-8");
    hsd::tuple t = {500, 'a', "more professional print"};
    hsd::io::print<L"this tuple contains: {}\n">(t);
    auto t2 = hsd::make_tuple(-1, static_cast<hsd::u64>(-1), 'c');
    auto t3 = t + t2 + hsd::make_tuple(L'ă', -1.32f, u8"柴尻保存会");
    hsd::io::print<L"this tuple contains: {}\n">(t3);
}