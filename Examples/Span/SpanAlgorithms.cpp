// Title: Span algorithms
// Level: Advanced
// Shows: generic algorithms over spans of different containers, enumerate, and constexpr spans.

#include <Containers/Span.hpp>
#include <Containers/StackArray.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Each algorithm is a template over the view type, so the same code works on
// a span of a vector, a stack_array or a const container.

template <typename View>
constexpr bool is_sorted(const View& view)
{
    bool first = true;
    hsd::i32 previous = 0;
    for (const auto& value : view)
    {
        if (!first && value < previous)
        {
            return false;
        }
        previous = value;
        first = false;
    }
    return true;
}

// enumerate() gives the position along with the element, so there is no need
// for a hand-written counter. The pair is [value, index].
template <typename View>
constexpr hsd::usize index_of_max(const View& view)
{
    hsd::usize best_index = 0;
    hsd::i32 best_value = 0;
    for (auto [value, index] : view.enumerate())
    {
        if (index == 0 || value > best_value)
        {
            best_value = value;
            best_index = index;
        }
    }
    return best_index;
}

template <typename View>
constexpr hsd::usize longest_ascending_run(const View& view)
{
    hsd::usize best = 0;
    hsd::usize current = 0;
    hsd::i32 previous = 0;
    for (const auto& value : view)
    {
        current = (current > 0 && value >= previous) ? current + 1 : 1;
        previous = value;
        best = current > best ? current : best;
    }
    return best;
}

// Merges two sorted views into a new vector, as in merge sort.
template <typename Left, typename Right>
hsd::vector<hsd::i32> merge_sorted(Left left, Right right)
{
    hsd::vector<hsd::i32> merged;
    merged.reserve(left.size() + right.size());

    while (left.size() > 0 && right.size() > 0)
    {
        if (*left.begin() <= *right.begin())
        {
            merged.push_back(*left.begin());
            left = left.drop(1).unwrap();
        }
        else
        {
            merged.push_back(*right.begin());
            right = right.drop(1).unwrap();
        }
    }
    for (const auto& value : left)
    {
        merged.push_back(value);
    }
    for (const auto& value : right)
    {
        merged.push_back(value);
    }
    return merged;
}

// The algorithms are constexpr, so they can run on a span during compilation.
constexpr hsd::usize compile_time_run()
{
    hsd::stack_array table{3, 1, 2, 4, 6, 5, 0};
    hsd::span view{table};
    return longest_ascending_run(view);
}
static_assert(compile_time_run() == 4);

constexpr hsd::usize compile_time_max_index()
{
    const hsd::stack_array table{3, 9, 2};
    hsd::span view{table};
    return index_of_max(view);
}
static_assert(compile_time_max_index() == 1);

template <typename View>
void report(const char* label, const View& view)
{
    hsd::print("{}:"_fmt, label);
    for (const auto& value : view)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
    hsd::println("  sorted: {}"_fmt, is_sorted(view) ? "true" : "false");
    hsd::println("  max at index: {}"_fmt, index_of_max(view));
    hsd::println("  longest ascending run: {}"_fmt, longest_ascending_run(view));
}

int main()
{
    hsd::vector<hsd::i32> from_vector{{2, 7, 7, 9, 1, 3, 4, 8, 0}};
    hsd::stack_array from_array{1, 3, 5, 7};
    const hsd::stack_array constant_array{6, 4, 2};

    report("vector", from_vector.to_span());
    report("stack_array", hsd::span{from_array});
    report("const stack_array", hsd::span{constant_array});

    // Two sorted views of different container types merge into one vector.
    hsd::vector<hsd::i32> evens{{0, 2, 4, 6, 8}};
    auto merged = merge_sorted(evens.to_span(), hsd::span{from_array});
    hsd::print("merged:"_fmt);
    for (const auto& value : merged)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);

    // A sub-view works like any other span: check only the sorted prefix.
    auto prefix = from_vector.to_span().take(4).unwrap();
    hsd::println("prefix of 4 sorted: {}"_fmt, is_sorted(prefix) ? "true" : "false");

    hsd::println("compile time run: {}"_fmt, compile_time_run());

    return 0;
}
