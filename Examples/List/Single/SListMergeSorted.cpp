// Title: Merging sorted lists
// Level: Advanced
// Shows: merge of two sorted forward_lists, a merge sort built on it, and de-duplication.

#include <Containers/ForwardList.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

using int_list = hsd::forward_list<hsd::i32>;

static void print_list(const char* label, const int_list& values)
{
    hsd::print("{}:"_fmt, label);
    for (const auto& value : values)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
}

// Merges two sorted lists into a new one. Only the heads are ever compared and
// removed, which are the cheap operations of a singly linked list. Taking from
// the left list on ties keeps the merge stable.
static int_list merge_sorted(int_list lhs, int_list rhs)
{
    int_list merged;

    while (lhs.size() != 0 && rhs.size() != 0)
    {
        if (rhs.front() < lhs.front())
        {
            merged.push_back(rhs.front());
            rhs.pop_front();
        }
        else
        {
            merged.push_back(lhs.front());
            lhs.pop_front();
        }
    }

    for (; lhs.size() != 0; lhs.pop_front())
    {
        merged.push_back(lhs.front());
    }

    for (; rhs.size() != 0; rhs.pop_front())
    {
        merged.push_back(rhs.front());
    }

    return merged;
}

// Splits the list by dealing elements alternately, sorts each half
// recursively, and merges them back. A linked list cannot be indexed, so
// dealing out the elements replaces the usual "cut at the middle".
static int_list merge_sort(const int_list& values)
{
    if (values.size() <= 1)
    {
        return values;
    }

    int_list left, right;
    bool to_left = true;

    for (const auto& value : values)
    {
        (to_left ? left : right).push_back(value);
        to_left = !to_left;
    }

    return merge_sorted(merge_sort(left), merge_sort(right));
}

static int_list unique_values(const int_list& sorted)
{
    int_list result;

    for (const auto& value : sorted)
    {
        if (result.size() == 0 || result.back() != value)
        {
            result.push_back(value);
        }
    }

    return result;
}

static bool is_sorted_list(const int_list& values)
{
    hsd::i32 previous = 0;
    bool first = true;

    for (const auto& value : values)
    {
        if (first == false && value < previous)
        {
            return false;
        }

        previous = value;
        first = false;
    }

    return true;
}

int main()
{
    int_list evens{{2, 4, 6, 8, 10}};
    int_list odds{{1, 3, 5, 7, 9, 11, 13}};
    print_list("merged", merge_sorted(evens, odds));

    // A stable generator: the same twenty numbers on every run.
    int_list noisy;
    hsd::u32 state = 12345;
    for (hsd::i32 count = 0; count < 20; ++count)
    {
        state = state * 1103515245u + 12345u;
        noisy.push_back(static_cast<hsd::i32>((state >> 16) % 15));
    }

    print_list("noisy ", noisy);
    int_list sorted = merge_sort(noisy);
    print_list("sorted", sorted);
    print_list("unique", unique_values(sorted));
    hsd::println("sorted ok: {}, same size: {}"_fmt,
        is_sorted_list(sorted) ? "true" : "false",
        sorted.size() == noisy.size() ? "true" : "false");

    // Several sorted runs fold into one by merging them pairwise.
    hsd::vector<int_list> runs;
    runs.push_back(int_list{{1, 4, 9}});
    runs.push_back(int_list{{2, 3, 10}});
    runs.push_back(int_list{{0, 5, 6, 7}});

    int_list combined;
    for (const auto& run : runs)
    {
        combined = merge_sorted(combined, run);
    }
    print_list("runs  ", combined);

    return 0;
}
