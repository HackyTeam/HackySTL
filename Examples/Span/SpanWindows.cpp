// Title: Span windows
// Level: Intermediate
// Shows: sliding windows and fixed-size chunks built from drop and take.

#include <Containers/Span.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename View>
hsd::i32 sum_of(const View& view)
{
    hsd::i32 total = 0;
    for (const auto& value : view)
    {
        total += value;
    }
    return total;
}

template <typename View>
hsd::i32 max_of(const View& view)
{
    hsd::i32 best = 0;
    bool first = true;
    for (const auto& value : view)
    {
        if (first || value > best)
        {
            best = value;
            first = false;
        }
    }
    return best;
}

int main()
{
    hsd::vector<hsd::i32> samples{{4, 8, 15, 16, 23, 42, 7, 3, 9}};
    auto view = samples.to_span();
    constexpr hsd::usize window = 3;

    // Window number i is drop(i) followed by take(window). A window never
    // copies elements, it is just a new pair of iterators.
    hsd::println("windows of {}:"_fmt, window);
    hsd::i32 best_sum = 0;
    hsd::usize best_start = 0;
    for (hsd::usize start = 0; start + window <= view.size(); ++start)
    {
        auto slice = view.drop(start).unwrap().take(window).unwrap();
        hsd::i32 total = sum_of(slice);
        hsd::println("  start {} sum {} max {}"_fmt, start, total, max_of(slice));

        if (start == 0 || total > best_sum)
        {
            best_sum = total;
            best_start = start;
        }
    }
    hsd::println("best window starts at {} with sum {}"_fmt, best_start, best_sum);

    // Chunks: peel `chunk_size` elements off the front until nothing is left.
    // The last chunk is shorter when the size does not divide evenly.
    constexpr hsd::usize chunk_size = 4;
    auto rest = view;
    hsd::usize chunk_number = 0;
    while (rest.size() > 0)
    {
        hsd::usize count = rest.size() < chunk_size ? rest.size() : chunk_size;
        auto chunk = rest.take(count).unwrap();

        hsd::print("chunk {}:"_fmt, chunk_number++);
        for (const auto& value : chunk)
        {
            hsd::print(" {}"_fmt, value);
        }
        hsd::println(" (sum {})"_fmt, sum_of(chunk));

        rest = rest.drop(count).unwrap();
    }

    // Windows see later changes, since they refer to the vector itself.
    auto first_window = view.take(window).unwrap();
    samples[0] = 100;
    hsd::println("first window sum after edit: {}"_fmt, sum_of(first_window));

    return 0;
}
