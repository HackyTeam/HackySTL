#include <cassert>
#include <string>

#include <Containers/Bitset.hpp>
#include <Containers/Integer.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/SStream.hpp>

using namespace hsd::format_literals;

// Regression tests for format tags that a value type cannot print, such as {hex} on
// a string. Such a misuse used to reach print_to at run time and panic. Now print_to
// accepts only the tags its value type can print, so the misuse is a compile error.
// The checks below ask the compiler whether a call with a tag is accepted, and the
// last check confirms that the supported tags still print the expected text.
namespace
{
    // Resolves piece `Index` of a format text, the way append_data does.
    template <auto Text, hsd::usize Index, hsd::usize Count>
    constexpr auto piece_format()
    {
        constexpr auto parsed = hsd::parse_literal<Text, Count>().unwrap();
        constexpr auto piece = parsed[Index];
        constexpr auto lit = hsd::format_literal<char, piece.length + 1>{
            .format = {piece.format, piece.length},
            .base = piece.base
        };
        return hsd::format<lit>();
    }

    constexpr auto none_fmt = piece_format<("{}"_fmt)(), 0, 2>();
    constexpr auto hex_fmt = piece_format<("{hex}"_fmt)(), 0, 2>();
    constexpr auto exp_fmt = piece_format<("{exp}"_fmt)(), 0, 2>();
    constexpr auto prec_fmt = piece_format<("{prec=2}"_fmt)(), 0, 2>();

    // Whether print_to accepts a value of type `Value` with the format `fmt`.
    template <auto fmt, typename Value>
    constexpr bool accepts = requires (Value&& value)
    {
        hsd::print_to<fmt, char>(nullptr, 0, hsd::forward<Value>(value));
    };
}

// Integers print none and hex, but not exponent or precision.
static_assert(accepts<none_fmt, int>);
static_assert(accepts<hex_fmt, int>);
static_assert(!accepts<exp_fmt, int>, "integers cannot print an exponent");
static_assert(!accepts<prec_fmt, int>, "integers cannot print a precision");

// Floating-point values print none, hex, exponent and precision.
static_assert(accepts<none_fmt, double>);
static_assert(accepts<hex_fmt, double>);
static_assert(accepts<exp_fmt, double>);
static_assert(accepts<prec_fmt, double>);

// Text prints with no presentation tag at all.
static_assert(accepts<none_fmt, hsd::string>);
static_assert(!accepts<hex_fmt, hsd::string>, "a string cannot print in hex");
static_assert(!accepts<exp_fmt, hsd::string>, "a string cannot print an exponent");
static_assert(!accepts<prec_fmt, hsd::string>, "a string cannot print a precision");
static_assert(accepts<none_fmt, hsd::string_view>);
static_assert(!accepts<hex_fmt, hsd::string_view>, "a string view cannot print in hex");
static_assert(accepts<none_fmt, const char*>);
static_assert(!accepts<hex_fmt, const char*>, "a C string cannot print in hex");

// Bitsets and big integers print none and hex.
static_assert(accepts<none_fmt, hsd::bitset<8>>);
static_assert(accepts<hex_fmt, hsd::bitset<8>>);
static_assert(!accepts<exp_fmt, hsd::bitset<8>>, "a bitset cannot print an exponent");
static_assert(!accepts<prec_fmt, hsd::bitset<8>>, "a bitset cannot print a precision");
static_assert(accepts<none_fmt, hsd::integer<64>>);
static_assert(accepts<hex_fmt, hsd::integer<64>>);
static_assert(!accepts<exp_fmt, hsd::integer<64>>, "a big integer cannot print an exponent");
static_assert(!accepts<prec_fmt, hsd::integer<64>>, "a big integer cannot print a precision");

namespace
{
    // The supported tags still print the expected text through a stream.
    void check_supported_tags_print()
    {
        hsd::sstream integer_hex;
        integer_hex.write_data("{hex}"_fmt, 255);
        assert(std::string(integer_hex.data(), integer_hex.size()) == "0xff");

        hsd::sstream float_precision;
        float_precision.write_data("{prec=2}"_fmt, 1.5);
        assert(std::string(float_precision.data(), float_precision.size()) == "1.50");

        hsd::sstream float_exponent;
        float_exponent.write_data("{exp}"_fmt, 1500.0);
        assert(std::string(float_exponent.data(), float_exponent.size()) == "1.500000e+03");

        // A bitset prints its whole 32-bit word, eight hex digits, with the 0x prefix.
        hsd::sstream bits_hex;
        bits_hex.write_data("{hex}"_fmt, hsd::bitset<8>{0b0011'0101});
        assert(std::string(bits_hex.data(), bits_hex.size()) == "0x00000035");

        hsd::sstream big_hex;
        big_hex.write_data("{hex}"_fmt, hsd::integer<64>{255});
        assert(std::string(big_hex.data(), big_hex.size()) == "0x00000000000000ff");
    }
}

int main()
{
    check_supported_tags_print();
    return 0;
}
