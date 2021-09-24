#include <Tuple.hpp>

int main()
{
    constexpr hsd::tuple t = {500, 'a', "more professional print"};
    constexpr auto t2 = hsd::make_tuple(-1, static_cast<hsd::u64>(-1), 'c');
    [[maybe_unused]] constexpr auto t3 = 
        t + t2 + hsd::make_tuple(L'ă', -1.32f, u8"柴尻保存会");
}