#include <cassert>

#include <Base/IntegerSequence.hpp>

using hsd::usize;

template <typename T, T... Values>
constexpr T sum_sequence(hsd::integer_sequence<T, Values...>)
{
    return (T{} + ... + Values);
}

template <usize... Values>
constexpr bool is_increasing(hsd::index_sequence<Values...>)
{
    constexpr usize values[] = {Values...};
    for (usize index = 1; index < sizeof...(Values); ++index)
    {
        if (values[index - 1] >= values[index])
        {
            return false;
        }
    }
    return true;
}

int main()
{
    using explicit_sequence = hsd::integer_sequence<unsigned, 9, 2, 5, 1>;
    static_assert(explicit_sequence::size() == 4);
    static_assert(sum_sequence(explicit_sequence{}) == 17);

    using indices = hsd::make_index_sequence<10>;
    static_assert(indices::size() == 10);
    static_assert(is_increasing(indices{}));

    using types = hsd::index_sequence_for<float, hsd::wchar, char>;
    static_assert(types::size() == 3);

    using ranged = hsd::ranged::make_index_sequence<5, 10>;
    static_assert(ranged::size() == 5);
    static_assert(sum_sequence(ranged{}) == 35);

    using inverse = hsd::inverse::make_index_sequence<5>;
    static_assert(inverse::size() == 5);
    static_assert(sum_sequence(inverse{}) == 10);

    assert(explicit_sequence::size() == 4);
}
