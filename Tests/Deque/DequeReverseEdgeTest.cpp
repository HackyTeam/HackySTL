#include <cassert>
#include <deque>
#include <type_traits>
#include <vector>

#include <Containers/Deque.hpp>

// Regression tests for reverse iteration over a deque. rbegin() must start at the
// last element and rend() must be reached after the first, for empty, wrapped and
// const deques. Before the fix, rbegin() and rend() were forward iterators.
namespace
{
    // Collects the values of a reverse walk. The walk stops once it has seen more
    // values than the deque holds, so a broken iterator fails the checks below
    // instead of looping forever.
    template <typename Deque>
    std::vector<int> reverse_values(Deque& values)
    {
        std::vector<int> seen;
        auto it = values.rbegin();
        const auto stop = values.rend();

        while (it != stop && seen.size() <= values.size())
        {
            seen.push_back(*it);
            ++it;
        }

        return seen;
    }

    void check_reverse_order()
    {
        hsd::deque<int> values{{1, 2, 3, 4}};
        const std::vector<int> expected{4, 3, 2, 1};
        assert(reverse_values(values) == expected);
    }

    void check_empty()
    {
        hsd::deque<int> empty;
        assert(empty.rbegin() == empty.rend());
        assert(reverse_values(empty).empty());
    }

    void check_wrapped_storage()
    {
        // Removing from the front and adding at the back moves the logical
        // first element away from the start of the ring storage.
        hsd::deque<int> values{{1, 2, 3, 4}};
        values.pop_front();
        values.pop_front();
        values.push_back(5);
        values.push_back(6);
        const std::vector<int> expected{6, 5, 4, 3};
        assert(reverse_values(values) == expected);
    }

    void check_long_wrapped_matches_std()
    {
        hsd::deque<int> values;
        std::deque<int> expected;

        for (int value = 0; value < 40; ++value)
        {
            values.push_back(value);
            expected.push_back(value);
        }

        for (int step = 0; step < 25; ++step)
        {
            values.pop_front();
            expected.pop_front();
        }

        for (int value = 40; value < 70; ++value)
        {
            values.push_back(value);
            expected.push_back(value);
        }

        const std::vector<int> reversed(expected.rbegin(), expected.rend());
        assert(reverse_values(values) == reversed);
    }

    void check_mutable_reverse()
    {
        // The reverse iterator refers to the stored element, so writes reach the deque.
        hsd::deque<int> values{{1, 2, 3}};
        *values.rbegin() = 30;
        assert(values.back() == 30);
    }

    void check_const_reverse()
    {
        // A const deque must give const access through its reverse iterators.
        const hsd::deque<int> values{{1, 2, 3}};
        static_assert(std::is_same_v<decltype(*values.rbegin()), const int&>);
        assert(*values.rbegin() == 3);
        const std::vector<int> expected{3, 2, 1};
        assert(reverse_values(values) == expected);
    }
}

int main()
{
    check_reverse_order();
    check_empty();
    check_wrapped_storage();
    check_long_wrapped_matches_std();
    check_mutable_reverse();
    check_const_reverse();
    return 0;
}
