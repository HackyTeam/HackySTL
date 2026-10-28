// Title: Gorbitsa hello
// Level: Basic
// Shows: running a Gorbitsa program at compile time with run_parser and reading its output as an array.

#include <Extra/Constexpr/Gorbitsa.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// A Gorbitsa program is a list of instructions written as a letter and a number.
// "S72" sets the accumulator to 72 (the code of 'H') and "T" prints the accumulator.
// The letters used here:
//     S n   put the number n in the accumulator
//     T     output the accumulator as one character
static constexpr char source[] = "S72 T S101 T S108 T T S111 T";

// The interpreter is a template, so the whole program runs while the compiler
// instantiates it. consteval forces the result to be known at compile time.
template <typename Program>
consteval auto run()
{
    return hsd::run_parser<Program, true, 0, 0>();
}

int main()
{
    // str_to_arr turns the string literal into a type that the interpreter can walk.
    constexpr auto output = run<typename hsd::str_to_arr<source>::type>();

    // The output exists at compile time, so it can be checked by static_assert.
    static_assert(output.size() == 5);
    static_assert(output[0] == 'H');
    static_assert(output[4] == 'o');

    hsd::print("output:"_fmt);

    for (auto code : output)
    {
        hsd::print(" {}"_fmt, static_cast<hsd::i32>(code));
    }

    hsd::println(""_fmt);

    hsd::print("text: "_fmt);

    for (auto code : output)
    {
        hsd::print("{}"_fmt, static_cast<char>(code));
    }

    hsd::println(""_fmt);
    return 0;
}
