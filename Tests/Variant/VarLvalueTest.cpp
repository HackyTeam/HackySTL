#include <cassert>

#include <Containers/String.hpp>
#include <Wrappers/Variant.hpp>

// Regression test: a variant built from an lvalue stores a copy of it.
int main()
{
    double value = 2.5;
    hsd::variant<double, hsd::string> from_lvalue{value};
    assert(from_lvalue.index() == 0);
    assert(from_lvalue.get<0>().unwrap().get() == 2.5);

    hsd::string text{"hi"};
    hsd::variant<double, hsd::string> from_text{text};
    assert(from_text.index() == 1);

    const hsd::i32 count = 3;
    hsd::variant<hsd::i32, hsd::string> from_const{count};
    assert(from_const.index() == 0);
    assert(from_const.get<0>().unwrap().get() == 3);
}
