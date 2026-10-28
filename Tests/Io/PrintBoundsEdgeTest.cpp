#include <cassert>
#include <memory>
#include <string>

#include <Containers/Bitset.hpp>
#include <Containers/Integer.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/SStream.hpp>

using namespace hsd::format_literals;

// Regression tests for print_to on a destination of exactly the size it is given.
// Every destination below is a heap block of exactly that many characters, so a
// write one character past the end is a heap-buffer-overflow under AddressSanitizer.
// A text needs one position after its characters for the terminator, so a block
// that holds only the characters must be reported as too small, not filled.
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

    constexpr auto literal_fmt = piece_format<("abcdefghijklmnop"_fmt)(), 0, 1>();
    constexpr auto number_fmt = piece_format<("{}"_fmt)(), 0, 2>();
    constexpr auto prefix_fmt = piece_format<("x{}"_fmt)(), 0, 2>();
    constexpr auto styled_fmt = piece_format<("{bold}"_fmt)(), 0, 2>();

    struct printed
    {
        bool ok = false;
        std::string text;
    };

    // Prints into a block of exactly `size` characters.
    template <auto fmt, typename... Args>
    printed print_into(hsd::usize size, Args&&... args)
    {
        std::unique_ptr<char[]> block{new char[size]};
        auto res = hsd::print_to<fmt, char>(block.get(), size, hsd::forward<Args>(args)...);

        if (res.is_ok() == false)
        {
            return {};
        }

        return {true, std::string(block.get(), res.unwrap())};
    }

    // Checks that `expected` prints into one more slot than it needs, and that
    // the block one slot short is refused.
    template <auto fmt>
    void expect_literal(const std::string& expected)
    {
        const printed fits = print_into<fmt>(expected.size() + 1);
        assert(fits.ok && fits.text == expected);
        assert(!print_into<fmt>(expected.size()).ok);
    }

    template <auto fmt, typename Value>
    void expect_value(const std::string& expected, const Value& value)
    {
        const printed fits = print_into<fmt>(expected.size() + 1, Value{value});
        assert(fits.ok && fits.text == expected);
        assert(!print_into<fmt>(expected.size(), Value{value}).ok);
    }

    void check_literal_only()
    {
        expect_literal<literal_fmt>("abcdefghijklmnop");
    }

    void check_integer()
    {
        expect_value<number_fmt>("1234567890123456", 1234567890123456);
        expect_value<number_fmt>("-42", -42);
    }

    void check_floating()
    {
        expect_value<number_fmt>("1.500000", 1.5);
    }

    void check_strings()
    {
        // The prefix "x" is written before the value, so the value must fit after it.
        expect_value<prefix_fmt>("xabcdefghijklmno", hsd::string{"abcdefghijklmno"});
        expect_value<number_fmt>("abcdefghijklmnop", hsd::string{"abcdefghijklmnop"});
        expect_value<number_fmt>("view", hsd::string_view{"view"});

        const hsd::static_string<32> fixed{"abcdefghijklmno"};
        expect_value<number_fmt>("abcdefghijklmno", fixed);
    }

    void check_bitset_and_big_integer()
    {
        const hsd::bitset<8> bits{0b0011'0101};
        expect_value<number_fmt>("00110101", bits);

        const hsd::integer<64> big = 1234567890123456ULL;
        expect_value<number_fmt>("1234567890123456", big);
    }

    void check_styled_value()
    {
        // A style adds an opening and a closing escape around the value, and both
        // count toward the room the text needs.
        const std::string open = std::string(styled_fmt.style.data);
        const std::string close = std::string(styled_fmt.reset.data);
        expect_value<styled_fmt>(open + "abcdef" + close, hsd::string{"abcdef"});
        expect_value<styled_fmt>(open + "7" + close, 7);
    }
}

int main()
{
    check_literal_only();
    check_integer();
    check_floating();
    check_strings();
    check_bitset_and_big_integer();
    check_styled_value();
    return 0;
}
