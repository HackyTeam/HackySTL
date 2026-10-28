// Title: Expense tracker
// Level: Intermediate
// Shows: a stdin command loop with result-based amount parsing, budgets and undo.

#include <Base/Result.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

struct expense
{
    hsd::string category;
    hsd::i32 cents;
};

struct tracker
{
    hsd::vector<expense> history;
    hsd::unordered_map<hsd::string, hsd::i32> spent;
    hsd::unordered_map<hsd::string, hsd::i32> limits;
};

// Prints cents as dollars, always with two decimals.
static void print_money(hsd::i32 cents)
{
    const hsd::i32 rest = cents % 100;
    hsd::print("{}.{}{}"_fmt, cents / 100, rest < 10 ? "0" : "", rest);
}

// Parses a non-negative whole number. Failure carries a stock error message.
static hsd::result<hsd::i32, hsd::runtime_error> parse_cents(const hsd::string& text)
{
    if (text.length() == 0)
    {
        return hsd::runtime_error{"missing amount"};
    }

    hsd::i32 value = 0;
    for (const char digit : text)
    {
        if (digit < '0' || digit > '9')
        {
            return hsd::runtime_error{"amount is not a number"};
        }
        value = value * 10 + (digit - '0');
    }
    return value;
}

static hsd::vector<hsd::string> split_fields(const hsd::string& line)
{
    hsd::vector<hsd::string> fields;
    hsd::string current;

    for (const char character : line)
    {
        if (character == ' ')
        {
            if (current.length() != 0)
            {
                fields.push_back(hsd::move(current));
                current.clear();
            }
        }
        else
        {
            current.push_back(character);
        }
    }

    if (current.length() != 0)
    {
        fields.push_back(hsd::move(current));
    }
    return fields;
}

static void print_report(tracker& state)
{
    hsd::i32 total = 0;
    for (const auto& entry : state.spent)
    {
        hsd::print("  {}: "_fmt, entry.first);
        print_money(entry.second);

        auto limit = state.limits.at(entry.first);
        if (limit.is_ok() && entry.second > limit.unwrap().get())
        {
            hsd::print(" (over budget by "_fmt);
            print_money(entry.second - limit.unwrap().get());
            hsd::print(")"_fmt);
        }
        hsd::println(""_fmt);
        total += entry.second;
    }
    hsd::print("  total: "_fmt);
    print_money(total);
    hsd::println(""_fmt);
}

static void run_command(tracker& state, const hsd::string& line)
{
    auto fields = split_fields(line);
    if (fields.size() == 0)
    {
        return;
    }

    if (fields[0] == "add"_sv && fields.size() == 3)
    {
        auto amount = parse_cents(fields[2]);
        if (!amount.is_ok())
        {
            hsd::println("error: {}"_fmt, amount.unwrap_err().pretty_error());
            return;
        }
        state.spent[fields[1]] += amount.unwrap();
        state.history.push_back(expense{fields[1], amount.unwrap()});
        hsd::println("added {}"_fmt, fields[1]);
    }
    else if (fields[0] == "budget"_sv && fields.size() == 3)
    {
        auto amount = parse_cents(fields[2]);
        if (!amount.is_ok())
        {
            hsd::println("error: {}"_fmt, amount.unwrap_err().pretty_error());
            return;
        }
        state.limits[fields[1]] = amount.unwrap();
        hsd::println("budget set for {}"_fmt, fields[1]);
    }
    else if (fields[0] == "undo"_sv && fields.size() == 1)
    {
        if (state.history.size() == 0)
        {
            hsd::println("error: nothing to undo"_fmt);
            return;
        }
        const expense last = state.history.back();
        state.history.pop_back();
        state.spent[last.category] -= last.cents;
        hsd::println("removed last {} expense"_fmt, last.category);
    }
    else if (fields[0] == "report"_sv && fields.size() == 1)
    {
        hsd::println("report:"_fmt);
        print_report(state);
    }
    else
    {
        hsd::println("error: unknown command '{}'"_fmt, fields[0]);
    }
}

// Reads all of stdin one line at a time with read_line(). Every line, including
// blank lines and a final line without a newline, is kept, and each one ends with
// '\n'. A single read can return only part of a pipe, so the loop is needed.
hsd::string read_script()
{
    hsd::string text;
    auto line = hsd::io::cin().read_line();

    while (line.is_ok())
    {
        text += line.unwrap();
        text += "\n";
        line = hsd::io::cin().read_line();
    }

    return text;
}

int main()
{
    const hsd::string script = read_script();

    tracker state;
    hsd::string line;
    for (const char character : script)
    {
        if (character == '\n')
        {
            run_command(state, line);
            line.clear();
        }
        else if (character != '\r')
        {
            line.push_back(character);
        }
    }
    run_command(state, line);
}
