#include <Wrappers/SharedPtr.hpp>
#include <cassert>

struct tracked
{
    static inline int destroyed = 0;
    int value;

    tracked(int v)
        : value{v}
    {}

    ~tracked()
    {
        ++destroyed;
    }
};

template <typename MakeShared>
static void test_copy_assign(MakeShared make)
{
    tracked::destroyed = 0;

    {
        auto first = make(1);
        auto second = make(2);
        const auto& source = first;
        second = source;

        assert(second->value == 1);
        assert(tracked::destroyed == 1);
    }

    assert(tracked::destroyed == 2);
}

int main()
{
    test_copy_assign([](int v) { return hsd::make_unsafe_shared<tracked>(hsd::move(v)); });
    test_copy_assign([](int v) { return hsd::make_safe_shared<tracked>(hsd::move(v)); });
    return 0;
}
