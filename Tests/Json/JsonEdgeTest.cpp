#include <cassert>
#include <cstdio>
#include <string>

#include <Extra/Json.hpp>

using namespace hsd::string_view_literals;

// The error paths of the JSON lexer and parser: characters and keywords that do not fit,
// numbers the lexer refuses, text that ends in the middle of an array or object, a file that
// cannot be read, and numbers outside the range of the type they are read as. Text that is
// split across several lex() calls is parsed as far as it goes, and reports where it stops.
namespace
{
    // Lexes `text`, adds the end-of-text token when `finish` is set, and parses one value.
    // Returns the error text, or an empty string when a value was parsed.
    std::string parse_error(hsd::string_view text, bool finish)
    {
        hsd::JsonStream lexer;
        auto lexed = lexer.lex(text);

        if (!lexed.is_ok())
        {
            auto error = lexed.unwrap_err();
            return std::string{error.pretty_error()};
        }

        if (finish)
        {
            auto ended = lexer.push_eot();

            if (!ended.is_ok())
            {
                auto error = ended.unwrap_err();
                return std::string{error.pretty_error()};
            }
        }

        hsd::JsonParser parser = lexer;
        auto parsed = parser.parse_next();

        if (parsed.is_ok())
        {
            return {};
        }

        auto error = parsed.unwrap_err();
        return std::string{error.pretty_error()};
    }

    // Lexes `text`, adds the end-of-text token, and parses one value that must be valid.
    hsd::unique_ptr<hsd::JsonValue> parse_value(hsd::string_view text)
    {
        hsd::JsonStream lexer;
        lexer.lex(text).unwrap();
        lexer.push_eot().unwrap();
        hsd::JsonParser parser = lexer;
        return parser.parse_next().unwrap();
    }

    // Text of a number with fifty digits before the point, which is far beyond the range of f32.
    std::string huge_number(bool negative)
    {
        std::string text = negative ? "-1" : "1";
        text.append(49, '0');
        text += ".0";
        return text;
    }
}

int main()
{
    // An error message is built once, and the same text is returned on later calls.
    {
        hsd::JsonError error{"Message", 7};
        assert(std::string{error.pretty_error()} == "Message 7");
        assert(std::string{error.pretty_error()} == "Message 7");
    }

    // The lexer refuses a character that starts no token, and a keyword with a wrong letter.
    assert(parse_error("@"_sv, true).starts_with("Syntax error: unexpected character"));
    assert(parse_error("nux"_sv, true).starts_with("Syntax error: unexpected character"));

    // A number has at most one point, and an exponent is not supported yet.
    assert(parse_error("1.2.3"_sv, true).starts_with("Syntax error: unexpected character"));
    assert(parse_error("1e5"_sv, true).starts_with("Work in progress: exponential notation"));
    assert(parse_error("1E5"_sv, true).starts_with("Work in progress: exponential notation"));

    // Text that ends too early is reported as partial, and where it stops.
    assert(parse_error(""_sv, true).starts_with("Unexpected EOF"));
    assert(parse_error("["_sv, false).starts_with("Work in progress: partial arrays"));
    assert(parse_error("[1 "_sv, false).starts_with("Work in progress: partial arrays"));
    assert(parse_error("{"_sv, false).starts_with("Work in progress: partial objects"));
    assert(parse_error("{\"a\""_sv, false).starts_with("Work in progress: partial objects"));
    assert(parse_error("{\"a\":1 , "_sv, false).starts_with("Work in progress: partial objects"));

    // Arrays and objects that are complete but badly separated are syntax errors.
    assert(parse_error("[1 2]"_sv, true).starts_with("Syntax error: expected a comma while parsing array"));
    assert(parse_error("{1:2}"_sv, true).starts_with("Syntax error: expected string name"));
    assert(parse_error("{\"a\" 1}"_sv, true).starts_with("Syntax error: expected a colon"));
    assert(parse_error("{\"a\":1 \"b\"}"_sv, true).starts_with("Syntax error: expected a comma while parsing object"));

    // A token that cannot begin a value is refused by the parser.
    assert(parse_error(","_sv, true).starts_with("Syntax error: unexpected token"));

    // Nothing is parsed from a stream that has not been lexed.
    {
        hsd::JsonStream lexer;
        hsd::JsonParser parser = lexer;
        auto parsed = parser.parse_next();
        assert(!parsed.is_ok());
        auto error = parsed.unwrap_err();
        assert(std::string{error.pretty_error()}.starts_with("Work in progress: pending values not supported"));
    }

    // A signed number parses, a number with a point and nothing after it reads as a float,
    // and a float at the end of the text is read when the text is finished.
    assert(parse_value("+5"_sv)->as_num<hsd::i32>().unwrap() == 5);
    assert(parse_value("-5"_sv)->as_num<hsd::i32>().unwrap() == -5);
    assert(parse_value("1. "_sv)->as_num<hsd::f32>().unwrap() == 1.0f);
    assert(parse_value("1.5"_sv)->as_num<hsd::f32>().unwrap() == 1.5f);

    // A value that is too large or too small for the requested type is refused.
    {
        const std::string big = huge_number(false);
        const std::string small = huge_number(true);
        auto positive = parse_value(hsd::string_view{big.c_str(), big.length()});
        auto negative = parse_value(hsd::string_view{small.c_str(), small.length()});
        assert(!positive->as_num<hsd::f32>().is_ok());
        assert(!negative->as_num<hsd::f32>().is_ok());
    }
    assert(!parse_value("3000000000"_sv)->as_num<hsd::i32>().is_ok());

    // A file that cannot be opened is reported at once.
    {
        hsd::JsonStream lexer;
        auto opened = lexer.lex_file("json_edge_missing_file.json"_sv);
        assert(!opened.is_ok());
        auto error = opened.unwrap_err();
        assert(std::string{error.pretty_error()}.starts_with("Couldn't open file"));
    }

    // A file is lexed through the same path as text. A file with a bad token reports it.
    {
        std::FILE* good = std::fopen("json_edge_good_file.json", "w");
        assert(good != nullptr);
        std::fputs("[1, 2.5]\n", good);
        std::fclose(good);

        std::FILE* bad = std::fopen("json_edge_bad_file.json", "w");
        assert(bad != nullptr);
        std::fputs("[1 @]\n", bad);
        std::fclose(bad);

        hsd::JsonStream lexer;
        auto lexed = lexer.lex_file("json_edge_good_file.json"_sv);
        assert(lexed.is_ok());
        assert(lexer.push_eot().is_ok());
        hsd::JsonParser parser = lexer;
        auto array = parser.parse_next().unwrap();
        assert(array->is_complete());

        hsd::JsonStream broken;
        auto failed = broken.lex_file("json_edge_bad_file.json"_sv);
        assert(!failed.is_ok());

        std::remove("json_edge_good_file.json");
        std::remove("json_edge_bad_file.json");
    }

    return 0;
}
