#include <IntegerSequence.hpp>
#include <Io.hpp>

using namespace hsd::format_literals;

static void print_num(size_t S)
{
    hsd::print("{} "_fmt, S);
}

template <typename T, T... ints>
static void print_sequence(hsd::integer_sequence<T, ints...> int_seq)
{
    hsd::print("The sequence of size {}: "_fmt, int_seq.size());
    
    (print_num(ints), ...);
    
    hsd::print("\n"_fmt);
}

int main()
{
    print_sequence(hsd::integer_sequence<unsigned, 9, 2, 5, 1, 9, 1, 6>{});
    print_sequence(hsd::make_integer_sequence<int, 20>{});
    print_sequence(hsd::make_index_sequence<10>{});
    print_sequence(hsd::index_sequence_for<float, hsd::wchar, char>{});
    print_sequence(hsd::ranged::make_index_sequence<5, 10>{});
    print_sequence(hsd::inverse::make_index_sequence<10>{});
    print_sequence(hsd::inverse::ranged::make_index_sequence<10, 5>{});
}