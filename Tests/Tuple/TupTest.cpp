#include "../../cpp/Io.hpp"
#include "../../cpp/Tuple.hpp"

template< typename... Args >
static void wprint(hsd::tuple<Args...> tup)
{
    [&]<hsd::usize... Ints>(hsd::index_sequence<Ints...>) {
        (hsd::io::wprint(L"{}\n", tup.template get<Ints>()), ...);
    }(hsd::make_index_sequence<tup.size()>{});
}

int main()
{
    setlocale(LC_ALL, "en_US.UTF-8");
    constexpr hsd::tuple t = {500, 'a', "more professional print"};
    wprint(t);
    hsd::io::wprint(L"\n");
    constexpr auto t2 = hsd::make_tuple(-1, (hsd::u64)-1, 'c');
    constexpr auto t3 = t + t2 + hsd::make_tuple(L'ă', -1.32f, L"柴尻保存会");
    wprint(t3);
}