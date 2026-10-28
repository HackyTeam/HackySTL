#include <cassert>
#include <memory>

#include <Wrappers/UniquePtr.hpp>

struct counted_value
{
    static inline int alive = 0;

    int value = 0;

    explicit counted_value(int initial_value = 0)
        : value{initial_value}
    {
        ++alive;
    }

    ~counted_value()
    {
        --alive;
    }
};

struct base
{
    virtual ~base() = default;
    virtual int value() const = 0;
};

struct derived : base
{
    int number;

    explicit derived(int initial_number)
        : number{initial_number}
    {}

    int value() const override
    {
        return number;
    }
};

static void test_single_object()
{
    auto first = hsd::make_unique<int>(11);
    auto standard_first = std::make_unique<int>(11);
    assert(*first == 11);
    assert(*first == *standard_first);

    hsd::unique_ptr<int> second{hsd::move(first)};
    std::unique_ptr<int> standard_second{std::move(standard_first)};
    assert(first == nullptr);
    assert(standard_first == nullptr);
    assert(*second == 11);
    assert(*second == *standard_second);

    second = hsd::make_unique<int>(22);
    standard_second = std::make_unique<int>(22);
    assert(*second == 22);
    assert(*second == *standard_second);
    second = nullptr;
    standard_second = nullptr;
    assert(second == nullptr);
    assert(standard_second == nullptr);

    hsd::unique_ptr<int> empty;
    std::unique_ptr<int> standard_empty;
    assert(empty == nullptr);
    assert(standard_empty == nullptr);
    empty = hsd::make_unique<int>(3);
    standard_empty = std::make_unique<int>(3);
    assert(*empty == 3);
    assert(*empty == *standard_empty);
}

static void test_arrays_and_lifetime()
{
    {
        auto values = hsd::make_unique<counted_value[]>(8);
        auto standard_values = std::make_unique<counted_value[]>(8);
        assert(counted_value::alive == 16);
        values.get()[3].value = 99;
        standard_values[3].value = 99;
        assert(values.get()[3].value == 99);
        assert(values.get()[3].value == standard_values[3].value);
    }

    assert(counted_value::alive == 0);
}

static void test_polymorphic_delete()
{
    hsd::unique_ptr<base> value = hsd::make_unique<derived>(42);
    std::unique_ptr<base> standard_value = std::make_unique<derived>(42);
    assert(value->value() == 42);
    assert(value->value() == standard_value->value());

    hsd::unique_ptr<base> moved{hsd::move(value)};
    std::unique_ptr<base> standard_moved{std::move(standard_value)};
    assert(value == nullptr);
    assert(standard_value == nullptr);
    assert(moved->value() == 42);
    assert(moved->value() == standard_moved->value());
    moved = nullptr;
    standard_moved = nullptr;
    assert(moved == nullptr);
    assert(standard_moved == nullptr);
}

static void test_buffered_allocator()
{
    hsd::uchar buffer[256]{};
    hsd::buffered_allocator alloc{buffer, sizeof(buffer)};
    auto values = hsd::make_unique<int[], hsd::buffered_allocator>(alloc, 6);
    for (int index = 0; index < 6; ++index)
    {
        values.get()[index] = index * index;
    }
    assert(values.get()[5] == 25);
}

int main()
{
    test_single_object();
    test_arrays_and_lifetime();
    test_polymorphic_delete();
    test_buffered_allocator();
}
