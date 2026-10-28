// Title: Stdin calculator
// Level: Intermediate
// Shows: reading all of stdin with io::cin, then parsing each command line with sstream::set_data.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/SStream.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

namespace
{
    // Evaluates "op a b". Returns false when the command cannot be run.
    bool evaluate(const hsd::string& op, hsd::f64 a, hsd::f64 b, hsd::f64& out)
    {
        if (op == hsd::string{"add"})
        {
            out = a + b;
        }
        else if (op == hsd::string{"sub"})
        {
            out = a - b;
        }
        else if (op == hsd::string{"mul"})
        {
            out = a * b;
        }
        else if (op == hsd::string{"div"})
        {
            if (b == 0)
            {
                hsd::println("division by zero"_fmt);
                return false;
            }

            out = a / b;
        }
        else
        {
            hsd::println("unknown operation {}"_fmt, op);
            return false;
        }

        return true;
    }
}

int main()
{
    // Read the input one line at a time. A single read can return only part of a
    // pipe, so the lines are collected until read_line reports the end of the input.
    // Each line gets its newline back so the text splits the same way as before.
    auto& input = hsd::io::cin();
    hsd::string input_text;
    auto raw_line = input.read_line();
    while (raw_line.is_ok())
    {
        input_text += raw_line.unwrap();
        input_text += "\n";
        raw_line = input.read_line();
    }
    hsd::string_view text{input_text.c_str(), input_text.length()};

    hsd::usize done = 0, failed = 0;
    hsd::f64 total = 0;

    for (hsd::usize start = 0; start < text.length(); )
    {
        hsd::usize end = text.find('\n', start);
        if (end == hsd::string_view::npos)
        {
            end = text.length();
        }

        hsd::string_view line = text.sub_string(start, end - start).unwrap();
        start = end + 1;

        if (line.length() == 0)
        {
            continue;
        }

        // set_data needs a separator after the last field, hence the space.
        hsd::sstream fields;
        fields.reserve(line.length() + 8);
        fields.write_data("{} "_fmt, line);

        hsd::string op;
        hsd::f64 a = 0, b = 0, result = 0;

        if (fields.set_data(op, a, b).is_ok() == false)
        {
            hsd::println("cannot read '{}'"_fmt, line);
            ++failed;
        }
        else if (evaluate(op, a, b, result))
        {
            hsd::println("{} {prec=2} {prec=2} = {prec=2}"_fmt, op, a, b, result);
            total += result;
            ++done;
        }
        else
        {
            ++failed;
        }
    }

    hsd::println("done: {}, failed: {}, total: {prec=2}"_fmt, done, failed, total);

    return 0;
}
