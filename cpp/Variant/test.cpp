#include "variant.hpp"

#include <iostream>

struct A {};
struct B {
    int a;
    constexpr B(int a) : a(a) {}
};

constexpr bool operator==(A, A) noexcept {return true; }
constexpr bool operator==(B a, B b) noexcept {return a.a == b.a; }

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

    std::cout << va1.get<0>() << std::endl;
    /*try {
        // This should throw -vvv
        std::cout << va1.get<float>() << std::endl;
    } catch (hsd::bad_variant_access& err) {
        std::cerr << "Caught error: " << err.what() << std::endl;
    }*/
    if (auto* val = hsd::get_if<float>(&va1)) {
        std::cout << "Got value: " << *val << std::endl;
    } else {
        std::cout << "No value of type float in va1" << std::endl;
    }
    if (auto* val = va2.get_if<B>()) {
        std::cout << "B inside va2: " << val->a << std::endl;
    } else {
        std::cout << "No B in va2" << std::endl;
    }

    std::cout << std::boolalpha;
    std::cout << "va2 == va3: " << (va2 == va3) << std::endl;
    va2 = va3;
    std::cout << "(after assignment) " << (va2 == va3) << std::endl;
    va3 = hsd::variant<A, B>(6);
    std::cout << "(after new assignment)" << std::endl << "va3 == va4: " << (va3 == va4) << std::endl;
    va3.get<B>().a = 5;
    std::cout << "(after modification) " << (va3 == va4) << std::endl;
}
