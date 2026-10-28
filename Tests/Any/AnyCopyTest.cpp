#include <Wrappers/Any.hpp>
#include <cassert>

struct Large
{
    long values[12];
};

int main()
{
    hsd::any empty;

    // An empty any has nothing to clone.
    hsd::any copy_of_empty{empty};
    assert(!copy_of_empty.has_value());

    copy_of_empty = empty;
    assert(!copy_of_empty.has_value());

    hsd::any moved_empty{hsd::move(empty)};
    assert(!moved_empty.has_value());

    // Small and large values copy independently.
    hsd::any small{41};
    hsd::any small_copy{small};
    *small_copy.cast_if<int>() += 1;
    assert(*small.cast_if<int>() == 41 && *small_copy.cast_if<int>() == 42);

    Large large_value{};
    large_value.values[11] = 7;

    hsd::any large{large_value};
    hsd::any large_copy{large};
    large_copy.cast_if<Large>()->values[11] = 8;
    assert(large.cast_if<Large>()->values[11] == 7);
    assert(large_copy.cast_if<Large>()->values[11] == 8);

    // Swap across empty, small and large holders.
    hsd::swap(small, large);
    assert(small.holds_type<Large>() && large.holds_type<int>());

    hsd::swap(small, empty);
    assert(!small.has_value() && empty.holds_type<Large>());

    // Assigning a different type replaces the value.
    empty = 3.5;
    assert(empty.holds_type<double>() && !empty.holds_type<Large>());

    return 0;
}
