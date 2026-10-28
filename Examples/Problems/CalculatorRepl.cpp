// Title: Calculator REPL
// Level: Advanced
// Shows: a recursive descent parser over stdin lines, with variables, functions and result errors.

#include <Base/Math.hpp>
#include <Base/Result.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

using value_result = hsd::result<hsd::f64, hsd::runtime_error>;
using variables = hsd::unordered_map<hsd::string, hsd::f64>;

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Grammar, lowest precedence first:
//   expr    = term (('+' | '-') term)*
//   term    = unary (('*' | '/') unary)*
//   unary   = '-' unary | power
//   power   = primary ('^' unary)?
//   primary = number | name | name '(' expr ')' | '(' expr ')'
// Every rule returns a result, so the first error travels up unchanged.
class parser
{
    hsd::string_view _text;
    hsd::usize _pos = 0;
    const variables& _vars;

    void skip_spaces()
    {
        while (_pos < _text.length() && _text[_pos] == ' ')
        {
            ++_pos;
        }
    }

    bool accept(char expected)
    {
        skip_spaces();
        if (_pos < _text.length() && _text[_pos] == expected)
        {
            ++_pos;
            return true;
        }
        return false;
    }

    value_result close_with(value_result inner)
    {
        if (inner.is_ok() && !accept(')'))
        {
            return hsd::runtime_error{"missing closing parenthesis"};
        }
        return inner;
    }

    value_result number()
    {
        hsd::f64 whole = 0;
        while (_pos < _text.length() && is_digit(_text[_pos]))
        {
            whole = whole * 10 + (_text[_pos++] - '0');
        }
        if (_pos < _text.length() && _text[_pos] == '.')
        {
            ++_pos;
            hsd::f64 scale = 0.1;
            while (_pos < _text.length() && is_digit(_text[_pos]))
            {
                whole += (_text[_pos++] - '0') * scale;
                scale /= 10;
            }
        }
        return whole;
    }

    value_result primary()
    {
        skip_spaces();
        if (_pos >= _text.length())
        {
            return hsd::runtime_error{"unexpected end of expression"};
        }

        if (is_digit(_text[_pos]))
        {
            return number();
        }

        if (accept('('))
        {
            return close_with(expr());
        }

        if (is_alpha(_text[_pos]))
        {
            hsd::string name;
            while (_pos < _text.length() && is_alpha(_text[_pos]))
            {
                name.push_back(_text[_pos++]);
            }

            if (accept('('))
            {
                auto argument = close_with(expr());
                if (!argument.is_ok())
                {
                    return argument;
                }

                const hsd::f64 x = argument.unwrap();
                if (name == "sqrt"_sv)
                {
                    if (x < 0)
                    {
                        return hsd::runtime_error{"square root of a negative number"};
                    }
                    return static_cast<hsd::f64>(hsd::math::sqrt(x));
                }
                return hsd::runtime_error{"unknown function"};
            }

            auto found = _vars.at(name);
            if (!found.is_ok())
            {
                return hsd::runtime_error{"unknown variable"};
            }
            return found.unwrap().get();
        }

        return hsd::runtime_error{"unexpected character"};
    }

    value_result power()
    {
        auto base = primary();
        if (!base.is_ok() || !accept('^'))
        {
            return base;
        }
        auto exponent = unary();
        if (!exponent.is_ok())
        {
            return exponent;
        }
        return static_cast<hsd::f64>(
            hsd::math::pow(base.unwrap(), exponent.unwrap()));
    }

    value_result unary()
    {
        if (accept('-'))
        {
            auto operand = unary();
            if (!operand.is_ok())
            {
                return operand;
            }
            return -operand.unwrap();
        }
        return power();
    }

    value_result term()
    {
        auto left = unary();
        while (left.is_ok())
        {
            const bool times = accept('*');
            if (!times && !accept('/'))
            {
                break;
            }

            auto right = unary();
            if (!right.is_ok())
            {
                return right;
            }
            if (!times && right.unwrap() == 0)
            {
                return hsd::runtime_error{"division by zero"};
            }
            left = times ? left.unwrap() * right.unwrap()
                : left.unwrap() / right.unwrap();
        }
        return left;
    }

public:
    parser(hsd::string_view text, const variables& vars)
        : _text{text}, _vars{vars}
    {}

    value_result expr()
    {
        auto left = term();
        while (left.is_ok())
        {
            const bool plus = accept('+');
            if (!plus && !accept('-'))
            {
                break;
            }

            auto right = term();
            if (!right.is_ok())
            {
                return right;
            }
            left = plus ? left.unwrap() + right.unwrap()
                : left.unwrap() - right.unwrap();
        }
        return left;
    }

    // After a full expression only spaces may remain.
    bool at_end()
    {
        skip_spaces();
        return _pos >= _text.length();
    }
};

static void evaluate_line(variables& vars, const hsd::string& line)
{
    // "name = expression" assigns, anything else just prints the value.
    hsd::string target;
    hsd::usize start = 0;
    const hsd::usize equals = line.find('=');
    if (equals != hsd::string::npos)
    {
        for (hsd::usize index = 0; index < equals; ++index)
        {
            if (line[index] != ' ')
            {
                target.push_back(line[index]);
            }
        }
        start = equals + 1;
    }

    hsd::string_view body{line.c_str() + start, line.length() - start};
    parser calc{body, vars};
    auto value = calc.expr();

    if (value.is_ok() && !calc.at_end())
    {
        hsd::println("error: unexpected trailing input"_fmt);
    }
    else if (!value.is_ok())
    {
        hsd::println("error: {}"_fmt, value.unwrap_err().pretty_error());
    }
    else if (target.length() != 0)
    {
        vars[target] = value.unwrap();
        hsd::println("{}: {prec=3}"_fmt, target, value.unwrap());
    }
    else
    {
        hsd::println("{prec=3}"_fmt, value.unwrap());
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

    variables vars;
    hsd::string line;
    for (const char character : script)
    {
        if (character == '\n')
        {
            if (line.length() != 0)
            {
                evaluate_line(vars, line);
            }
            line.clear();
        }
        else if (character != '\r')
        {
            line.push_back(character);
        }
    }
    if (line.length() != 0)
    {
        evaluate_line(vars, line);
    }
}
