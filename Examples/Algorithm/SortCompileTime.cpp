// Title: Sorting at compile time
// Level: Advanced
// Shows: constexpr sorts checked by static_assert, and sorters that cover only a sub-range.

#include <Base/Algorithm.hpp>
#include <Containers/StackArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

constexpr hsd::usize count = 12;
using numbers = hsd::stack_array<hsd::u32, count>;

// Builds a scrambled array. Multiplying by 7 modulo 12 visits every value once.
constexpr numbers make_input()
{
    numbers values;

    for (hsd::u32 index = 0; index < count; index++)
    {
        values[index] = (index * 7 + 3) % count;
    }

    return values;
}

constexpr bool is_ascending(const numbers& values)
{
    for (hsd::usize index = 1; index < count; index++)
    {
        if (values[index - 1] > values[index])
        {
            return false;
        }
    }

    return true;
}

// Each sorter runs inside constant evaluation, on its own copy of the input.
constexpr numbers sorted_by_radix()
{
    numbers values = make_input();
    hsd::radix_sort(values);
    return values;
}

constexpr numbers sorted_by_heap()
{
    numbers values = make_input();
    hsd::heap_sort(values);
    return values;
}

constexpr numbers sorted_by_intro()
{
    numbers values = make_input();
    hsd::introsort(values);
    return values;
}

// The range sorters take an inclusive [low, high] pair and leave the rest alone.
constexpr numbers middle_sorted()
{
    numbers values = make_input();
    hsd::insertion_sorter<numbers>::sort(values, 3, 8);
    return values;
}

constexpr numbers middle_sorted_descending()
{
    numbers values = make_input();
    auto pred = [](hsd::u32 lhs, hsd::u32 rhs) { return lhs < rhs; };
    hsd::heap_sorter<numbers>::sort(values, pred, 3, 8);
    return values;
}

static_assert(!is_ascending(make_input()));
static_assert(is_ascending(sorted_by_radix()));
static_assert(is_ascending(sorted_by_heap()));
static_assert(is_ascending(sorted_by_intro()));
static_assert(middle_sorted()[2] == make_input()[2]);
static_assert(middle_sorted()[9] == make_input()[9]);

template <typename Vec>
void print_all(const char* label, const Vec& values)
{
    hsd::print("{}:"_fmt, label);
    for (hsd::usize index = 0; index < count; index++)
    {
        hsd::print(" {}"_fmt, values[index]);
    }
    hsd::println(""_fmt);
}

int main()
{
    // These constants were computed by the compiler. Printing them shows
    // the results of the static_asserts above.
    constexpr numbers input = make_input();
    constexpr numbers radix = sorted_by_radix();
    constexpr numbers heap = sorted_by_heap();
    constexpr numbers intro = sorted_by_intro();
    constexpr numbers middle = middle_sorted();
    constexpr numbers middle_desc = middle_sorted_descending();

    print_all("input", input);
    print_all("radix", radix);
    print_all("heap", heap);
    print_all("intro", intro);
    print_all("indices 3..8 ascending", middle);
    print_all("indices 3..8 descending", middle_desc);

    return 0;
}
