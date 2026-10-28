// Title: Argument parser basics
// Level: Basic
// Shows: registering options with argument_parser and parsing a prepared argv.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/ArgParse.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

int main()
{
    // The parser keeps a description, and each add() keeps a help line.
    hsd::argument_parser parser = "Demo of the argument parser"_sv;

    hsd::i32 number = 0;
    hsd::string text = hsd::string{"none"};

    // An option names how many tokens follow it. The callback receives them in
    // a parser_stream, and set_data converts each one to the variable's type.
    parser.add(
        "-number", 1,
        [&number](hsd::parser_stream& stream)
        {
            stream.set_data(number).unwrap();
        },
        "-number: an integer"_sv
    );

    parser.add(
        "-text", 1,
        [&text](hsd::parser_stream& stream)
        {
            stream.set_data(text).unwrap();
        },
        "-text: a word"_sv
    );

    // Index 0 is the program name, as in a real main.
    const char* argv[] = {"program", "-number", "42", "-text", "hello"};
    parser.parse(5, argv);
    hsd::println("number: {}, text: {}"_fmt, number, text);

    // Options may come in any order, and unmentioned ones keep their value.
    const char* second[] = {"program", "-number", "-7"};
    parser.parse(3, second);
    hsd::println("number: {}, text: {}"_fmt, number, text);

    // Hexadecimal tokens are understood too.
    const char* hex[] = {"program", "-number", "0x2a"};
    parser.parse(3, hex);
    hsd::println("number: {}, text: {}"_fmt, number, text);

    // With only the program name there is nothing to do.
    const char* bare[] = {"program"};
    parser.parse(1, bare);
    hsd::println("number: {}, text: {}"_fmt, number, text);

    return 0;
}
