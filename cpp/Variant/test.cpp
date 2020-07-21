#include "variant.hpp"

#include <iostream>

struct A {};
struct B {
    B(int) {}
};

int main() {
    hsd::variant<int, float> va1(5);
    hsd::variant<A, B> va2(5);
    hsd::variant<A, B> va3{A()};
    auto va4(va2);

    using ty1 = hsd::variant_alternative<0, decltype(va1)>;
    using ty2 = hsd::variant_alternative<1, decltype(va3)>;
    std::cout << hsd::variant_size<decltype(va4)>::value << std::endl;

    std::cout << va1.index() << std::endl;
    std::cout << va2.index() << std::endl;
    std::cout << va3.index() << std::endl;
    std::cout << va4.index() << std::endl;
}
