// Title: Expanding arrays with index sequences
// Level: Intermediate
// Shows: turning an array into a pack of arguments, ranged and inverse sequences.

#include <Base/IntegerSequence.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int volume(int width, int height, int depth)
{
    return width * height * depth;
}

// An index pack lets one fold touch every array element, so the array can be
// passed on as separate arguments.
template <hsd::usize N, hsd::usize... indices>
int call_with_array(const int (&values)[N], hsd::index_sequence<indices...>)
{
    return volume(values[indices]...);
}

template <hsd::usize N>
int call_with_array(const int (&values)[N])
{
    return call_with_array(values, hsd::make_index_sequence<N>{});
}

template <hsd::usize N, hsd::usize... indices>
void print_selected(const char* label, const int (&values)[N], hsd::index_sequence<indices...>)
{
    hsd::print("{}:"_fmt, label);
    (hsd::print(" {}"_fmt, values[indices]), ...);
    hsd::println(""_fmt);
}

int main()
{
    const int box[3] = {2, 3, 4};
    hsd::println("volume={}"_fmt, call_with_array(box));

    const int data[8] = {10, 20, 30, 40, 50, 60, 70, 80};

    // Indices 2 up to, but not including, 5.
    print_selected("ranged 2..5", data, hsd::ranged::make_index_sequence<2, 5>{});

    // Counting down from 4 to 0.
    print_selected("inverse 5", data, hsd::inverse::make_index_sequence<5>{});

    // Indices 6 down to 3, from the ranged inverse builder.
    print_selected("inverse ranged 7..3", data, hsd::inverse::ranged::make_index_sequence<7, 3>{});

    return 0;
}
