// Title: Stack of a forward list
// Level: Intermediate
// Shows: push_front/pop_front as a stack, in an RPN calculator reading stdin.

#include <Containers/ForwardList.hpp>
#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// The head of a singly linked list is the only cheap place to add and remove,
// which is exactly what a stack needs: push_front, front, pop_front.
class int_stack
{
    hsd::forward_list<hsd::i64> _items;

public:
    void push(hsd::i64 value)
    {
        _items.push_front(value);
    }

    // Takes the top value, or reports false when the stack is empty.
    bool pop(hsd::i64& out)
    {
        if (_items.size() == 0)
        {
            return false;
        }

        out = _items.front();
        _items.pop_front();
        return true;
    }

    hsd::usize size() const
    {
        return _items.size();
    }
};

static bool is_digit(char value)
{
    return value >= '0' && value <= '9';
}

// Evaluates one line such as "3 4 + 2 *". Returns an empty message on success.
static const char* evaluate(const hsd::string& line, hsd::i64& answer)
{
    int_stack stack;
    hsd::usize index = 0;

    while (index <= line.length())
    {
        // Gather one token up to the next space.
        hsd::string token;
        for (; index < line.length() && line[index] != ' '; ++index)
        {
            token.push_back(line[index]);
        }
        ++index;

        if (token.length() == 0)
        {
            continue;
        }

        if (is_digit(token[0]))
        {
            hsd::i64 number = 0;
            for (hsd::usize digit = 0; digit < token.length(); ++digit)
            {
                if (is_digit(token[digit]) == false)
                {
                    return "bad number";
                }
                number = number * 10 + (token[digit] - '0');
            }
            stack.push(number);
        }
        else if (token.length() == 1 && (token[0] == '+' || token[0] == '-'
            || token[0] == '*' || token[0] == '/'))
        {
            hsd::i64 rhs = 0, lhs = 0;
            if (stack.pop(rhs) == false || stack.pop(lhs) == false)
            {
                return "not enough operands";
            }

            switch (token[0])
            {
                case '+': stack.push(lhs + rhs); break;
                case '-': stack.push(lhs - rhs); break;
                case '*': stack.push(lhs * rhs); break;
                default:
                    if (rhs == 0)
                    {
                        return "division by zero";
                    }
                    stack.push(lhs / rhs);
                    break;
            }
        }
        else
        {
            return "unknown token";
        }
    }

    if (stack.size() != 1 || stack.pop(answer) == false)
    {
        return "expression must leave one value";
    }

    return "";
}

int main()
{
    // The stream hands over a line once more input follows it, so the input
    // finishes with an "end" line.
    for (auto line = hsd::io::cin().read_line(); line.is_ok();
        line = hsd::io::cin().read_line())
    {
        const hsd::string text = hsd::move(line.unwrap());

        if (text == hsd::string{"end"})
        {
            break;
        }

        hsd::i64 answer = 0;
        const char* problem = evaluate(text, answer);

        if (problem[0] == '\0')
        {
            hsd::println("{} = {}"_fmt, text, answer);
        }
        else
        {
            hsd::println("{} -> error: {}"_fmt, text, problem);
        }
    }

    return 0;
}
