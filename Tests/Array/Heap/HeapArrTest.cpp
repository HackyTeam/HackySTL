#include <array>
#include <cassert>

#include <Containers/HeapArray.hpp>
#include <Support/TestSupport.hpp>

int main()
{
    hsd::heap_array<int, 5> values{{1, 2, 3, 4, 5}};
    std::array<int, 5> expected{{1, 2, 3, 4, 5}};

    assert(values.size() == expected.size());
    test_support::expect_sequence(values, expected);
    assert(values.at(0).unwrap().get() == 1);
    assert(values.at(4).unwrap().get() == 5);
    assert(!values.at(5).is_ok());

    values[2] = 30;
    expected[2] = 30;
    test_support::expect_sequence(values, expected);

    hsd::heap_array<int, 5> copy{values};
    test_support::expect_sequence(copy, expected);
    values[0] = 100;
    assert(copy[0] == 1);

    hsd::heap_array<int, 5> assigned;
    assigned = values;
    assert(assigned[0] == 100);

    hsd::heap_array<int, 5> moved{hsd::move(values)};
    assert(moved[0] == 100);
    assigned = hsd::move(moved);
    assert(assigned[2] == 30);
}
