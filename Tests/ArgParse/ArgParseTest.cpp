#include <cassert>
#include <Serialize/ArgParse.hpp>
#include <Containers/String.hpp>

int main()
{
    using namespace hsd::string_view_literals;
    hsd::argument_parser arg_parser = "This is a test"_sv;
    int parsed_number = 0;
    hsd::string parsed_text;

    arg_parser.add(
        "-number", 1,
        [&parsed_number](hsd::parser_stream& stream)
        {
            stream.set_data(parsed_number).unwrap();
        },
        "-number: Parses an integer"
    );

    arg_parser.add(
        "-text", 1, [&parsed_text](hsd::parser_stream& stream)
        {
            stream.set_data(parsed_text).unwrap();
        }, "-text: Parses a string"
    );

    const char* argv[] = {"program", "-number", "42", "-text", "hello"};
    arg_parser.parse(5, argv);

    assert(parsed_number == 42);
    assert(parsed_text == hsd::string{"hello"});
    arg_parser.parse(1, argv);

    const char* updated_argv[] = {"program", "-number", "7"};
    arg_parser.parse(3, updated_argv);
    assert(parsed_number == 7);
    assert(parsed_text == hsd::string{"hello"});

    hsd::parser_stream stream;
    int parsed_hex = 0;
    stream.emplace("0x2a");
    assert(stream.set_data(parsed_hex).is_ok());
    assert(parsed_hex == 42);

    stream.clear();
    stream.emplace("12z");
    parsed_hex = 9;
    assert(!stream.set_data(parsed_hex).is_ok());
    assert(parsed_hex == 9);

    stream.clear();
    assert(!stream.set_data(parsed_hex).is_ok());
}