// Title: Integer sequences
// Level: Basic
// Shows: integer_sequence, make_index_sequence, and reading the values back with a fold.

#include <Base/IntegerSequence.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// The values of the sequence are template arguments, so a function taking the
// sequence by type can unpack them as a parameter pack.
template <typename T, T... values>
void print_sequence(const char* label, hsd::integer_sequence<T, values...>)
{
    hsd::print("{}:"_fmt, label);
    (hsd::print(" {}"_fmt, values), ...);
    hsd::println(""_fmt);
}

template <typename T, T... values>
constexpr T sum_sequence(hsd::integer_sequence<T, values...>)
{
    return (T{} + ... + values);
}

int main()
{
    // Written out by hand.
    using handmade = hsd::integer_sequence<int, 9, 2, 5, 1>;
    print_sequence("handmade", handmade{});
    hsd::println("{} values, total {}"_fmt, handmade::size(), sum_sequence(handmade{}));

    // Generated: the indices 0 up to, but not including, 6.
    print_sequence("indices", hsd::make_index_sequence<6>{});

    // One index for each type in a list.
    print_sequence("for three types", hsd::index_sequence_for<char, float, long>{});

    return 0;
}
