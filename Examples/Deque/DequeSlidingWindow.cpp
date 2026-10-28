// Title: Deque sliding window maximum
// Level: Intermediate
// Shows: a monotonic deque that keeps window maxima, using front, back and both pops.

#include <initializer_list>

#include <Containers/Deque.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Returns the maximum of every window of `width` consecutive values.
//
// The deque stores indices, and the values at those indices always decrease
// from front to back. The front is therefore the maximum of the current
// window. Each index is pushed once and popped once, so the whole scan is
// linear.
hsd::vector<hsd::i32> window_maxima(const hsd::vector<hsd::i32>& data, hsd::usize width)
{
    hsd::vector<hsd::i32> result;
    hsd::deque<hsd::usize> candidates;

    for (hsd::usize index = 0; index < data.size(); ++index)
    {
        // The front index fell out of the window on the left.
        if (candidates.size() > 0 && candidates.front() + width <= index)
        {
            candidates.pop_front();
        }

        // Smaller values behind the new one can never be a maximum again.
        while (candidates.size() > 0 && data[candidates.back()] <= data[index])
        {
            candidates.pop_back();
        }
        candidates.push_back(index);

        if (index + 1 >= width)
        {
            result.push_back(data[candidates.front()]);
        }
    }

    return result;
}

// The simple version, used to double-check the deque version.
hsd::vector<hsd::i32> brute_force(const hsd::vector<hsd::i32>& data, hsd::usize width)
{
    hsd::vector<hsd::i32> result;
    for (hsd::usize start = 0; start + width <= data.size(); ++start)
    {
        hsd::i32 best = data[start];
        for (hsd::usize offset = 1; offset < width; ++offset)
        {
            if (data[start + offset] > best)
            {
                best = data[start + offset];
            }
        }
        result.push_back(best);
    }
    return result;
}

int main()
{
    hsd::vector<hsd::i32> data{{1, 3, -1, -3, 5, 3, 6, 7, 2, 2, 9, 1}};

    for (hsd::usize width : {1, 3, 4, 12})
    {
        auto fast = window_maxima(data, width);
        auto slow = brute_force(data, width);

        hsd::print("width {}:"_fmt, width);
        for (const auto& value : fast)
        {
            hsd::print(" {}"_fmt, value);
        }
        hsd::println(""_fmt);

        bool same = fast.size() == slow.size();
        for (hsd::usize idx = 0; same && idx < fast.size(); ++idx)
        {
            same = fast[idx] == slow[idx];
        }
        hsd::println("  matches brute force: {}"_fmt, same ? "true" : "false");
    }

    return 0;
}
