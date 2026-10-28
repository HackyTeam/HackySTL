#include <cassert>

#include <Containers/String.hpp>
#include <Wrappers/Pair.hpp>
#include <Wrappers/Tuple.hpp>

// Regression tests: tie with several references, make_tuple from const lvalues,
// and equality of pairs of ordinary types.
int main()
{
    hsd::i32 first = 1;
    hsd::i32 second = 2;
    auto refs = hsd::tie(first, second);
    refs.get<0>() = 10;
    refs.get<1>() = 20;
    assert(first == 10 && second == 20);

    const hsd::tuple<hsd::i32> source{7};
    auto copied = hsd::make_tuple(source, hsd::i32{3});
    assert(copied.get<1>() == 3);
    assert(copied.get<0>().get<0>() == 7);

    hsd::pair<hsd::i32, char> left{1, 'a'};
    hsd::pair<hsd::i32, char> same{1, 'a'};
    hsd::pair<hsd::i32, char> other{2, 'a'};
    assert(left == same);
    assert(!(left == other));
    assert(left != other);

    hsd::pair<hsd::string, hsd::i32> named{hsd::string{"x"}, 1};
    hsd::pair<hsd::string, hsd::i32> named_same{hsd::string{"x"}, 1};
    assert(named == named_same);
}
