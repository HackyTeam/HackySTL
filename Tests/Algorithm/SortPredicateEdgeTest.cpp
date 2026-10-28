#include <cassert>

#include <Base/Algorithm.hpp>
#include <Containers/Vector.hpp>

// Regression tests for sorts with a predicate: the element type needs no
// operator>, because the predicate supplies the order.
namespace
{
    struct record
    {
        int key;
        int id;
    };

    bool by_key(const record& lhs, const record& rhs)
    {
        return lhs.key > rhs.key;
    }

    template <typename Sorter>
    void check_sorted(Sorter sorter)
    {
        hsd::vector<record> values{{record{4, 0}, record{1, 1}, record{3, 2}, record{1, 3}, record{2, 4}}};
        sorter(values);

        // `pred(a, b)` true puts `a` after `b`, so `a.key > b.key` gives ascending keys.
        for (hsd::usize index = 1; index < values.size(); ++index)
        {
            assert(values[index - 1].key <= values[index].key);
        }
        assert(values.size() == 5);
    }
}

int main()
{
    check_sorted([](auto& values) { hsd::quick_sort(values, by_key); });
    check_sorted([](auto& values) { hsd::insertion_sort(values, by_key); });
    check_sorted([](auto& values) { hsd::heap_sort(values, by_key); });

    hsd::vector<int> numbers{{3, 1, 2}};
    hsd::quick_sort(numbers);
    assert(numbers[0] == 1 && numbers[1] == 2 && numbers[2] == 3);
}
