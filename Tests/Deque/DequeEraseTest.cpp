#include <Containers/Deque.hpp>
#include <cassert>
#include <initializer_list>

struct counted
{
    static inline int live = 0;
    int value;

    counted(int v)
        : value{v}
    {
        ++live;
    }

    counted(const counted& other)
        : value{other.value}
    {
        ++live;
    }

    counted(counted&& other)
        : value{other.value}
    {
        ++live;
    }

    counted& operator=(const counted&) = default;
    counted& operator=(counted&&) = default;

    ~counted()
    {
        --live;
    }
};

static void test_middle_range()
{
    hsd::deque<counted> d;

    for (int v : {1, 2, 3, 4, 5})
    {
        d.push_back(counted{v});
    }

    auto from = d.begin();
    ++from;

    auto to = from;
    ++to;
    ++to;

    d.erase_for(from, to).unwrap();

    assert(d.size() == 3);
    assert(counted::live == 3);
    assert(d[0].value == 1 && d[1].value == 4 && d[2].value == 5);

    d.push_back(counted{9});
    assert(d.size() == 4 && d[3].value == 9);
    assert(counted::live == 4);
}

static void test_suffix_range()
{
    hsd::deque<counted> d;

    for (int v : {1, 2, 3, 4, 5})
    {
        d.push_back(counted{v});
    }

    auto from = d.begin();
    ++from;
    ++from;
    ++from;

    d.erase_for(from, d.end()).unwrap();

    assert(d.size() == 3);
    assert(counted::live == 3);
    assert(d[0].value == 1 && d[2].value == 3);
}

static void test_iterator_equality()
{
    hsd::deque<int> a;
    hsd::deque<int> b;
    a.push_back(1);
    b.push_back(1);

    assert(!(a.begin() == b.begin()));
    assert(a.begin() != b.begin());
    assert(a.begin() == a.begin());
}

int main()
{
    test_middle_range();
    test_suffix_range();
    assert(counted::live == 0);

    test_iterator_equality();
    return 0;
}
