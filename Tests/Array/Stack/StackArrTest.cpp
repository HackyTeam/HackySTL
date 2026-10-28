#include <array>
#include <cassert>

#include <Containers/StackArray.hpp>
#include <Support/TestSupport.hpp>

int main()
{
    hsd::stack_array values{1, 2, 3, 4, 5};
    std::array<int, 5> expected{{1, 2, 3, 4, 5}};

    static_assert(decltype(values)::size() == 5);
    test_support::expect_sequence(values, expected);
    assert(values.at(0).unwrap().get() == 1);
    assert(values.at(4).unwrap().get() == 5);
    assert(!values.at(5).is_ok());

    values.fill(9);
    expected.fill(9);
    test_support::expect_sequence(values, expected);

    values[2] = 4;
    expected[2] = 4;
    test_support::expect_sequence(values, expected);

    hsd::stack_array<int, 5> other;
    other.fill(1);
    hsd::swap(values, other);
    assert(values[0] == 1);
    assert(other[2] == 4);
}
