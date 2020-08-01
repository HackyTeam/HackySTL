#include <iostream>
#include "../../cpp/Tuple.hpp"

template< typename... Args >
static void print(hsd::tuple<Args...> tup)
{
    [&]<size_t... Ints>(hsd::index_sequence<Ints...>){
        ((std::cout << tup.template get<Ints>() << '\n'), ...);
    }(hsd::make_index_sequence<tup.size()>{});
}

int main()
{
    hsd::tuple t = {500, 'a', "more professional print"};
    print(t);
    auto t2 = hsd::make_tuple(1, -1, 'c');
    print(t + hsd::make_tuple(1, -1, 'c'));
}