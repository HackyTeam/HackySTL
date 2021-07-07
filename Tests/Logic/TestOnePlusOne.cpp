#include <Extra/Constexpr/BinaryArithmetic.hpp>
#include <Extra/Constexpr/BooleanSerialize.hpp>
#include <Io.hpp>

template <hsd::BinaryNumber v>
auto serialize_bitset() 
{
    return hsd::serialize_value_v<v>.bs();
}

int main() 
{
    static_assert(
        hsd::is_same<
            hsd::ripple_adder<
                hsd::natural::one, 
                hsd::natural::one, 
                hsd::bit::zero
            >, hsd::natural::two
        >::value
    );

    hsd::io::print<"natural::one is {}\n">(
        serialize_bitset<hsd::natural::one>()
        .to_string().c_str()
    );
    hsd::io::print<"natural::two is {}\n">(
        serialize_bitset<hsd::natural::two>()
        .to_string().c_str()
    );
    hsd::io::print<"natural::three is {}\n">(
        serialize_bitset<hsd::natural::three>()
        .to_string().c_str()
    );
    hsd::io::print<"natural::four is {}\n">(
        serialize_bitset<hsd::natural::four>()
        .to_string().c_str()
    );

    hsd::io::print<"natural::two + natural::four is {}\n">(
        serialize_bitset<
            hsd::ripple_adder<
                hsd::natural::two, 
                hsd::natural::four, 
                hsd::bit::zero
            >
        >().to_string().c_str()
    );

    hsd::io::print<"5 + natural::four is {}\n">(
        serialize_bitset<hsd::ripple_adder<
            hsd::cons_pair<
                hsd::bit::one, hsd::cons_pair<
                    hsd::bit::zero, hsd::cons_pair<
                        hsd::bit::one, hsd::nil
                    >
                >
            >, hsd::natural::four, hsd::bit::zero>
        >().to_string().c_str()
    );
}
