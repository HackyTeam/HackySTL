#include "../../cpp/Io.hpp"
#include "../../cpp/Tuple.hpp"

template< typename... Args >
static void wprint(hsd::tuple<Args...> tup)
{
    [&]<size_t... Ints>(hsd::index_sequence<Ints...>){
        (hsd::io::wprint(L"{}\n" ,tup.template get<Ints>()), ...);
    }(hsd::make_index_sequence<tup.size()>{});
}

int main()
{
    setlocale(LC_ALL, "ro_RO.UTF-8");
    hsd::tuple t = {500, 'a', "more professional print"};
    wprint(t);
    hsd::io::wprint(L"\n");
    auto t2 = hsd::make_tuple(1, -1, 'c');
    wprint(t + t2 + hsd::make_tuple(L'ă', -1u, 'c'));
}