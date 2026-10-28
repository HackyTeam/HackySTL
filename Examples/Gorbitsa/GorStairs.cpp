// Title: Gorbitsa stairs
// Level: Intermediate
// Shows: generating Gorbitsa source with a constexpr function, then using S, T and I to draw a staircase of letters.

#include <Extra/Constexpr/Gorbitsa.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Instructions used here, each written as a letter and a number:
//     S n   put the number n in the accumulator
//     T     output the accumulator as one character
//     I n   add n to the accumulator (modulo 256)
// So "S65 T I1 T I1 T" outputs 'A', then 'B', then 'C'.

// A tiny fixed-size text builder that works in constant expressions.
template <hsd::usize Capacity>
struct program_text
{
    char data[Capacity]{};
    hsd::usize length = 0;

    constexpr void add(const char* text)
    {
        for (; *text != '\0'; ++text)
        {
            data[length++] = *text;
        }
    }
};

// Row k starts from 'A' and steps one letter at a time, then ends with a newline.
// Rows are separated by a space because the language needs one between instructions.
constexpr auto build_stairs(hsd::usize rows)
{
    program_text<512> text;

    for (hsd::usize row = 1; row <= rows; row++)
    {
        if (row > 1)
            text.add(" ");

        text.add("S65 T");

        for (hsd::usize step = 1; step < row; step++)
        {
            text.add(" I1 T");
        }

        text.add(" S10 T");
    }

    return text;
}

// The string must have static storage so the interpreter can take its address.
static constexpr auto stairs = build_stairs(5);

template <typename Program>
consteval auto run()
{
    return hsd::run_parser<Program, true, 0, 0>();
}

int main()
{
    constexpr auto output = run<typename hsd::str_to_arr<stairs.data>::type>();

    // Rows of 1 to 5 letters plus a newline each: 1+2+3+4+5 + 5 characters.
    static_assert(output.size() == 20);
    static_assert(output[0] == 'A');
    static_assert(output[1] == '\n');

    hsd::println("program length: {}"_fmt, stairs.length);

    for (auto code : output)
    {
        hsd::print("{}"_fmt, static_cast<char>(code));
    }

    hsd::println("output length: {}"_fmt, output.size());
    return 0;
}
