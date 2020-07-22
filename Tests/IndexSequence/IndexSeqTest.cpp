#include <iostream>
#include "../cpp/IndexSequence/IndexSequence.hpp"

static void print(size_t S)
{
    std::cout << S << ' ';
}

template<typename T, T... ints>
static void print_sequence(hsd::integer_sequence<T, ints...> int_seq)
{
    std::cout << "The sequence of size " << int_seq.size() << ": ";
    (print(ints), ...);
    std::cout << '\n';
}

int main()
{
    print_sequence(hsd::integer_sequence<unsigned, 9, 2, 5, 1, 9, 1, 6>{});
    print_sequence(hsd::make_integer_sequence<int, 20>{});
    print_sequence(hsd::make_index_sequence<10>{});
    print_sequence(hsd::index_sequence_for<float, std::iostream, char>{});
}