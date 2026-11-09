#include <cassert>
#include <cmath>

#include <Serialize/ArgParse.hpp>
#include <Serialize/SStream.hpp>

using namespace hsd::format_literals;

// Parsing text into typed values: decimal and hexadecimal integers, decimal, exponent and
// hexadecimal floating-point literals, single characters and character arrays, and the
// separator rules of a string stream that reads several values from one line. The argument
// stream takes one token per value; the string stream takes one line of separated values.
namespace
{
    // Parses one token through the argument stream.
    template <typename T>
    bool parse_token(const char* text, T& value)
    {
        hsd::parser_stream stream;
        stream.emplace(text);
        return stream.set_data(value).is_ok();
    }

    // Reads two integers from one line through the string stream.
    bool pair_from(const char* text, int& first, int& second)
    {
        hsd::sstream stream;
        stream.write_data("{}"_fmt, text);
        return stream.set_data(first, second).is_ok();
    }

    bool near(double value, double expected)
    {
        return std::fabs(value - expected) < 1e-12;
    }
}

int main()
{
    // Integers: decimal with an optional sign, and hexadecimal with either prefix case and
    // either digit case.
    int number = 0;
    assert(parse_token("42", number) && number == 42);
    assert(parse_token("-12", number) && number == -12);
    assert(parse_token("+7", number) && number == 7);
    assert(parse_token("0x2a", number) && number == 42);
    assert(parse_token("0X2A", number) && number == 42);
    assert(parse_token("0xFF", number) && number == 255);
    assert(!parse_token("", number));
    assert(!parse_token("12z", number));
    assert(!parse_token("1.5", number));
    assert(!parse_token("0x", number));
    assert(!parse_token("0xZZ", number));

    // Floats in decimal: a plain fraction, a sign, and an exponent in either letter case.
    double real = 0.0;
    assert(parse_token("1.5", real) && real == 1.5);
    assert(parse_token("+1.5", real) && real == 1.5);
    assert(parse_token("-2.25", real) && real == -2.25);
    assert(parse_token("1e3", real) && real == 1000.0);
    assert(parse_token("2.5E-2", real) && near(real, 0.025));
    assert(!parse_token("1..5", real));
    assert(!parse_token("1.5q", real));
    assert(!parse_token("1e", real));
    assert(!parse_token("1e3x", real));
    assert(!parse_token("", real));

    // Floats in hexadecimal: the mantissa digits count in sixteenths after the point, and the
    // binary exponent scales by powers of two. Both prefix letters and both digit cases work.
    assert(parse_token("0x1.8p1", real) && near(real, 3.0));
    assert(parse_token("0X1.8P1", real) && near(real, 3.0));
    assert(parse_token("0x0.Cp0", real) && near(real, 0.75));
    assert(parse_token("0x0.cp0", real) && near(real, 0.75));
    assert(parse_token("0x1.8AP1", real) && near(real, 3.078125));
    assert(parse_token("0x1p-2", real) && near(real, 0.25));
    assert(!parse_token("0x1.8", real));
    assert(!parse_token("0x1p", real));
    assert(!parse_token("0x1.8.8p0", real));
    assert(!parse_token("0xZ.8p0", real));
    assert(!parse_token("0x1.8Pq", real));

    // A single character, and a character array that holds the whole text with its terminator.
    char letter = '?';
    assert(parse_token("x", letter) && letter == 'x');
    assert(!parse_token("", letter));

    char word[4] = {'?', '?', '?', '?'};
    assert(parse_token("hey", word));
    assert(word[0] == 'h' && word[1] == 'e' && word[2] == 'y' && word[3] == '\0');
    char short_word[4] = {'?', '?', '?', '?'};
    assert(!parse_token("hello", short_word));
    assert(!parse_token("", word));

    // The string stream reads one value per field. Fields are split on spaces, tabs and
    // newlines, and a trailing separator completes the last field.
    int first = -1;
    int second = -1;
    assert(pair_from("5 6 ", first, second) && first == 5 && second == 6);
    assert(pair_from(" 5\t6\n", first, second) && first == 5 && second == 6);
    assert(pair_from("5 6 7", first, second) && first == 5 && second == 6);
    assert(!pair_from("5 6", first, second));
    assert(!pair_from("5 ", first, second));
    assert(!pair_from("   ", first, second));
    assert(!pair_from("", first, second));
    assert(!pair_from("5 x ", first, second));

    // Too few tokens for the outputs is reported, and a failed conversion stops the rest.
    hsd::parser_stream short_stream;
    short_stream.emplace("1");
    int lone = 0;
    int missing = 0;
    assert(!short_stream.set_data(lone, missing).is_ok());

    hsd::parser_stream failing_stream;
    failing_stream.emplace("x");
    failing_stream.emplace("2");
    int bad = 0;
    int after = 0;
    assert(!failing_stream.set_data(bad, after).is_ok());
    assert(after == 0);

    return 0;
}
