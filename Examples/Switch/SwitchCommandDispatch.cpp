// Title: Command dispatch
// Level: Advanced
// Shows: switch_cases as a command interpreter for a small stack machine, with nested switches and constexpr use.

#include <Base/Switch.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

// switch_cases is constexpr, so it can run at compile time as well.
constexpr hsd::i32 weekday_number(hsd::string_view name)
{
    hsd::i32 number = 0;

    hsd::switch_cases(
        name,
        hsd::equal_case{"mon", [&] { number = 1; }},
        hsd::equal_case{"tue", [&] { number = 2; }},
        hsd::equal_case{"wed", [&] { number = 3; }},
        hsd::equal_case{"thu", [&] { number = 4; }},
        hsd::equal_case{"fri", [&] { number = 5; }},
        hsd::default_case{[&] { number = 0; }}
    );

    return number;
}

static_assert(weekday_number("wed"_sv) == 3);
static_assert(weekday_number("sun"_sv) == 0);

class stack_machine
{
private:
    hsd::vector<hsd::i32> _stack;
    bool _running = true;

    // Handlers cannot hand an error back to switch_cases (it would abort),
    // so problems are printed where they happen.
    bool _need(hsd::usize count)
    {
        if (_stack.size() < count)
        {
            hsd::println("  error: stack needs {} values"_fmt, count);
            return false;
        }
        return true;
    }

    void _binary(char op)
    {
        if (!_need(2)) return;

        hsd::i32 right = _stack.back();
        _stack.pop_back();
        hsd::i32 left = _stack.back();
        _stack.pop_back();
        hsd::i32 result = 0;

        // A nested switch picks the arithmetic from the operator character.
        hsd::switch_cases(
            op,
            hsd::equal_case{'+', [&] { result = left + right; }},
            hsd::equal_case{'-', [&] { result = left - right; }},
            hsd::equal_case{'*', [&] { result = left * right; }},
            hsd::default_case{[&] { result = 0; }}
        );

        _stack.push_back(result);
    }

public:
    bool running() const
    {
        return _running;
    }

    void execute(hsd::string_view command, hsd::i32 argument)
    {
        hsd::switch_cases(
            command,
            hsd::equal_case{"push", [&] { _stack.push_back(argument); }},
            hsd::equal_case{"add", [&] { _binary('+'); }},
            hsd::equal_case{"sub", [&] { _binary('-'); }},
            hsd::equal_case{"mul", [&] { _binary('*'); }},
            hsd::equal_case{"dup", [&]
            {
                if (_need(1)) _stack.push_back(_stack.back());
            }},
            hsd::equal_case{"pop", [&]
            {
                if (_need(1)) _stack.pop_back();
            }},
            hsd::equal_case{"print", [&]
            {
                if (_need(1)) hsd::println("  top = {}"_fmt, _stack.back());
            }},
            hsd::equal_case{"halt", [&] { _running = false; }},
            hsd::default_case{[&] { hsd::println("  error: unknown command"_fmt); }}
        );
    }
};

struct instruction
{
    hsd::string_view command;
    hsd::i32 argument;
};

int main()
{
    hsd::println("wed={} fri={} sun={}"_fmt,
        weekday_number("wed"_sv), weekday_number("fri"_sv), weekday_number("sun"_sv));

    // (3 + 4) * 5, then square the result, with a few mistakes on the way.
    const instruction program[]{
        {"push"_sv, 3}, {"push"_sv, 4}, {"add"_sv, 0}, {"push"_sv, 5},
        {"mul"_sv, 0}, {"print"_sv, 0}, {"dup"_sv, 0}, {"mul"_sv, 0},
        {"print"_sv, 0}, {"jump"_sv, 0}, {"pop"_sv, 0}, {"pop"_sv, 0},
        {"halt"_sv, 0}, {"push"_sv, 99}
    };

    stack_machine machine;
    for (const auto& step : program)
    {
        if (!machine.running())
        {
            break;
        }

        hsd::println("{}"_fmt, step.command);
        machine.execute(step.command, step.argument);
    }

    hsd::println("machine stopped: {}"_fmt, machine.running() ? "false" : "true");
    return 0;
}
