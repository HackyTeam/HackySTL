#include <cassert>
#include <bitset>

#include <Containers/Bitset.hpp>

template <std::size_t Size>
static void expect_same_bits(
    const hsd::bitset<Size>& actual,
    const std::bitset<Size>& expected)
{
    assert(actual.size() == expected.size());
    assert(actual.count() == expected.count());
    assert(actual.all() == expected.all());
    assert(actual.any() == expected.any());
    assert(actual.none() == expected.none());
    for (std::size_t index = 0; index < Size; ++index)
    {
        assert(actual.test(index).unwrap() == expected.test(index));
    }
}

static void test_whole_set_operations()
{
    hsd::bitset<8> value{0b0011'0101};
    std::bitset<8> expected{0b0011'0101};
    expect_same_bits(value, expected);
    assert(value.size() == 8);
    assert(value.count() == 4);
    assert(value.any());
    assert(!value.none());
    assert(!value.all());

    value.set();
    expected.set();
    expect_same_bits(value, expected);
    assert(value.all());
    assert(value.count() == 8);

    value.reset();
    expected.reset();
    expect_same_bits(value, expected);
    assert(value.none());
    assert(value.count() == 0);

    value.flip();
    expected.flip();
    expect_same_bits(value, expected);
    assert(value == 0xff);
    value.flip();
    expected.flip();
    expect_same_bits(value, expected);
    assert(value == expected.to_ulong());
    assert(value == 0);
}

static void test_individual_bits()
{
    hsd::bitset<10> value;
    std::bitset<10> expected;
    for (hsd::usize index = 0; index < value.size(); ++index)
    {
        assert(!value.test(index).unwrap());
        value.set(index, index % 2 == 0).unwrap();
        expected.set(index, index % 2 == 0);
    }
    expect_same_bits(value, expected);

    for (hsd::usize index = 0; index < value.size(); ++index)
    {
        assert(value.test(index).unwrap() == (index % 2 == 0));
    }

    assert(!value.test(10).is_ok());
    assert(!value.set(10).is_ok());
    value.reset(9);
    expected.reset(9);
    value.flip(1);
    expected.flip(1);
    expect_same_bits(value, expected);
    assert(value.test(1).unwrap());
}

static void test_shifts_and_constexpr()
{
    constexpr hsd::bitset<4> initial{0b1011};
    static_assert(initial.any());
    static_assert(initial.count() == 3);

    hsd::bitset<4> value{initial};
    value >>= 1;
    assert(value == 0b0101);
    value <<= 2;
    assert(value == 0b0100);
}

int main()
{
    test_whole_set_operations();
    test_individual_bits();
    test_shifts_and_constexpr();
}
