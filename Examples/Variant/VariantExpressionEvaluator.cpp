// Title: Expression evaluator
// Level: Advanced
// Shows: variants as token and result types, a recursive descent parser, and visit over stdin expressions.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Variant.hpp>

using namespace hsd::format_literals;

// Tokens are one of three unrelated types, which is what a variant is for.
struct number_token
{
    hsd::f64 value;
};

struct operator_token
{
    char symbol;
};

struct paren_token
{
    bool opens;
};

using token = hsd::variant<number_token, operator_token, paren_token>;

// An evaluation gives either a number or an error message.
using outcome = hsd::variant<hsd::f64, hsd::string>;

static outcome fail(const char* message)
{
    return outcome{hsd::string{message}};
}

static bool is_failed(const outcome& value)
{
    return value.holds<hsd::string>();
}

// Cuts the text into tokens. On a bad character the error comes back in `error`.
static bool tokenize(const hsd::string& text, hsd::vector<token>& tokens, hsd::string& error)
{
    hsd::usize index = 0;

    while (index < text.length())
    {
        char current = text[index];

        if (current == ' ')
        {
            ++index;
        }
        else if ((current >= '0' && current <= '9') || current == '.')
        {
            hsd::f64 whole = 0.0;
            hsd::f64 scale = 0.1;
            bool in_fraction = false;

            for (; index < text.length(); ++index)
            {
                current = text[index];

                if (current == '.' && !in_fraction)
                {
                    in_fraction = true;
                }
                else if (current >= '0' && current <= '9')
                {
                    if (in_fraction)
                    {
                        whole += (current - '0') * scale;
                        scale /= 10.0;
                    }
                    else
                    {
                        whole = whole * 10.0 + (current - '0');
                    }
                }
                else
                {
                    break;
                }
            }

            tokens.push_back(token{number_token{whole}});
        }
        else if (current == '+' || current == '-' || current == '*' || current == '/')
        {
            tokens.push_back(token{operator_token{current}});
            ++index;
        }
        else if (current == '(' || current == ')')
        {
            tokens.push_back(token{paren_token{current == '('}});
            ++index;
        }
        else
        {
            error = hsd::string{"unexpected character '"};
            error.push_back(current);
            error.push_back('\'');
            return false;
        }
    }

    return true;
}

// Grammar:  sum := product (('+' | '-') product)*
//           product := factor (('*' | '/') factor)*
//           factor := number | '-' factor | '(' sum ')'
class parser
{
private:
    hsd::vector<token>& _tokens;
    hsd::usize _position = 0;

    // The next token as an operator symbol, or '\0' when it is another kind.
    char peek_operator()
    {
        if (_position < _tokens.size())
        {
            if (auto* op = _tokens[_position].get_if<operator_token>())
            {
                return op->symbol;
            }
        }

        return '\0';
    }

    bool peek_paren(bool opens)
    {
        if (_position < _tokens.size())
        {
            if (auto* paren = _tokens[_position].get_if<paren_token>())
            {
                return paren->opens == opens;
            }
        }

        return false;
    }

public:
    explicit parser(hsd::vector<token>& tokens)
        : _tokens{tokens}
    {}

    bool at_end() const
    {
        return _position >= _tokens.size();
    }

    outcome factor()
    {
        if (at_end())
        {
            return fail("unexpected end of expression");
        }

        if (peek_operator() == '-')
        {
            ++_position;
            outcome inner = factor();

            if (is_failed(inner))
            {
                return inner;
            }

            return outcome{-inner.get<hsd::f64>().unwrap().get()};
        }

        if (peek_paren(true))
        {
            ++_position;
            outcome inner = sum();

            if (is_failed(inner))
            {
                return inner;
            }

            if (!peek_paren(false))
            {
                return fail("missing ')'");
            }

            ++_position;
            return inner;
        }

        if (auto* literal = _tokens[_position].get_if<number_token>())
        {
            ++_position;
            // Build from a prvalue: the converting constructor does not accept lvalues.
            return outcome{hsd::f64{literal->value}};
        }

        return fail("unexpected token");
    }

    outcome product()
    {
        outcome left = factor();

        while (!is_failed(left) && (peek_operator() == '*' || peek_operator() == '/'))
        {
            char symbol = peek_operator();
            ++_position;
            outcome right = factor();

            if (is_failed(right))
            {
                return right;
            }

            hsd::f64 divisor = right.get<hsd::f64>().unwrap().get();

            if (symbol == '/' && divisor == 0.0)
            {
                return fail("division by zero");
            }

            hsd::f64 current = left.get<hsd::f64>().unwrap().get();
            left = outcome{symbol == '*' ? current * divisor : current / divisor};
        }

        return left;
    }

    outcome sum()
    {
        outcome left = product();

        while (!is_failed(left) && (peek_operator() == '+' || peek_operator() == '-'))
        {
            char symbol = peek_operator();
            ++_position;
            outcome right = product();

            if (is_failed(right))
            {
                return right;
            }

            hsd::f64 current = left.get<hsd::f64>().unwrap().get();
            hsd::f64 other = right.get<hsd::f64>().unwrap().get();
            left = outcome{symbol == '+' ? current + other : current - other};
        }

        return left;
    }
};

static outcome evaluate(const hsd::string& text)
{
    hsd::vector<token> tokens;
    hsd::string error;

    if (!tokenize(text, tokens, error))
    {
        return outcome{hsd::move(error)};
    }

    parser expression{tokens};
    outcome answer = expression.sum();

    // Leftover tokens, such as an unmatched ')', are an error too.
    if (!is_failed(answer) && !expression.at_end())
    {
        return fail("unexpected token after expression");
    }

    return answer;
}

int main()
{
    hsd::i32 good = 0;
    hsd::i32 bad = 0;

    // Each line is one expression. The stream hands over a line only when more
    // input follows it, so the input finishes with an "end" line that stops the loop.
    for (auto line = hsd::io::cin().read_line(); line.is_ok();
        line = hsd::io::cin().read_line())
    {
        hsd::string text = hsd::move(line.unwrap());

        if (text == hsd::string{"end"})
        {
            break;
        }

        if (text.length() == 0)
        {
            continue;
        }

        outcome answer = evaluate(text);

        // visit picks the printing for each alternative.
        answer.visit([&](const auto& value)
        {
            using type = hsd::remove_cvref_t<decltype(value)>;

            if constexpr (hsd::IsSame<type, hsd::f64>)
            {
                hsd::println("{} => {prec=2}"_fmt, text.c_str(), value);
                ++good;
            }
            else
            {
                hsd::println("{} => error: {}"_fmt, text.c_str(), value.c_str());
                ++bad;
            }
        }).unwrap();
    }

    hsd::println("ok: {} errors: {}"_fmt, good, bad);
    return 0;
}
