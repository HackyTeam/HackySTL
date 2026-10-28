// Title: Argument parser callbacks
// Level: Intermediate
// Shows: flags, multi-token options, repeated options and error reporting from callbacks.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/ArgParse.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

namespace
{
    // Everything the callbacks change lives here, so each run can start fresh.
    struct calculator
    {
        hsd::f32 value = 0;
        hsd::usize operations = 0;
        bool trace = false;
        hsd::i32 errors = 0;
        hsd::i32 range_lo = 0;
        hsd::i32 range_hi = 0;
    };

    // Reads one number from the stream. A bad token is counted, not fatal.
    bool read_number(hsd::parser_stream& stream, calculator& calc, hsd::f32& out)
    {
        if (stream.set_data(out).is_ok() == false)
        {
            ++calc.errors;
            return false;
        }

        return true;
    }

    void note(const calculator& calc, const char* what, hsd::f32 operand)
    {
        if (calc.trace)
        {
            hsd::println("  {} {} -> {}"_fmt, what, operand, calc.value);
        }
    }

    void run(const char* title, int argc, const char** argv)
    {
        calculator calc;
        hsd::argument_parser parser = "Calculator driven by options"_sv;

        // A flag takes zero tokens.
        parser.add(
            "-trace", 0,
            [&calc](hsd::parser_stream&) { calc.trace = true; },
            "-trace: print each step"_sv
        );

        // Options run in the order they appear, so repeating one is fine.
        parser.add(
            "-add", 1,
            [&calc](hsd::parser_stream& stream)
            {
                hsd::f32 operand = 0;
                if (read_number(stream, calc, operand))
                {
                    calc.value += operand;
                    ++calc.operations;
                    note(calc, "add", operand);
                }
            },
            "-add N: add N"_sv
        );

        parser.add(
            "-mul", 1,
            [&calc](hsd::parser_stream& stream)
            {
                hsd::f32 operand = 0;
                if (read_number(stream, calc, operand))
                {
                    calc.value *= operand;
                    ++calc.operations;
                    note(calc, "mul", operand);
                }
            },
            "-mul N: multiply by N"_sv
        );

        // An option may take several tokens, read together in one set_data call.
        parser.add(
            "-range", 2,
            [&calc](hsd::parser_stream& stream)
            {
                if (stream.set_data(calc.range_lo, calc.range_hi).is_ok() == false)
                {
                    ++calc.errors;
                }
            },
            "-range LO HI: clamp bounds"_sv
        );

        parser.parse(argc, argv);

        // Apply the range last, whatever the order on the command line was.
        if (calc.range_lo < calc.range_hi)
        {
            hsd::f32 lo = static_cast<hsd::f32>(calc.range_lo);
            hsd::f32 hi = static_cast<hsd::f32>(calc.range_hi);
            calc.value = calc.value < lo ? lo : (calc.value > hi ? hi : calc.value);
        }

        hsd::println("{}: value {prec=1}, operations {}, errors {}"_fmt,
            title, calc.value, calc.operations, calc.errors);
    }
}

int main()
{
    const char* plain[] = {"calc", "-add", "5", "-mul", "3", "-add", "-2"};
    run("plain", 7, plain);

    const char* traced[] = {"calc", "-trace", "-add", "1.5", "-mul", "4"};
    run("traced", 6, traced);

    const char* clamped[] = {"calc", "-add", "100", "-range", "0", "50"};
    run("clamped", 6, clamped);

    // A token that is not a number is counted and skipped.
    const char* broken[] = {"calc", "-add", "ten", "-add", "4"};
    run("broken", 5, broken);

    return 0;
}
