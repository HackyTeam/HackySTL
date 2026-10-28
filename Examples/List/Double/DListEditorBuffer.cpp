// Title: Editor buffer with a cursor
// Level: Intermediate
// Shows: two doubly linked lists around a cursor, driven by commands read from stdin.

#include <Containers/List.hpp>
#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// The text is split at the cursor: `before` holds the characters to its left
// (the last one is nearest the cursor) and `after` the ones to its right.
// Moving the cursor shifts one character between the lists in O(1), and so
// does typing or deleting at it.
class editor_buffer
{
    hsd::list<char> _before;
    hsd::list<char> _after;

public:
    void type(char value)
    {
        _before.push_back(value);
    }

    void backspace()
    {
        if (_before.empty() == false)
        {
            _before.pop_back();
        }
    }

    void erase_forward()
    {
        if (_after.empty() == false)
        {
            _after.pop_front();
        }
    }

    void move_left(hsd::usize count)
    {
        for (; count != 0 && _before.empty() == false; --count)
        {
            _after.push_front(_before.back());
            _before.pop_back();
        }
    }

    void move_right(hsd::usize count)
    {
        for (; count != 0 && _after.empty() == false; --count)
        {
            _before.push_back(_after.front());
            _after.pop_front();
        }
    }

    hsd::string text() const
    {
        hsd::string result;
        for (const char value : _before)
        {
            result.push_back(value);
        }
        for (const char value : _after)
        {
            result.push_back(value);
        }
        return result;
    }

    hsd::usize cursor() const
    {
        return _before.size();
    }
};

static hsd::usize parse_count(const hsd::string& line, hsd::usize from)
{
    hsd::usize count = 0;
    for (hsd::usize index = from; index < line.length(); ++index)
    {
        if (line[index] >= '0' && line[index] <= '9')
        {
            count = count * 10 + static_cast<hsd::usize>(line[index] - '0');
        }
    }
    return count;
}

int main()
{
    editor_buffer buffer;

    // Commands: type <text>, left <n>, right <n>, back, del, show, end.
    // The stream hands over a line once more input follows it, so the input
    // finishes with an "end" line.
    for (auto line = hsd::io::cin().read_line(); line.is_ok();
        line = hsd::io::cin().read_line())
    {
        const hsd::string command = hsd::move(line.unwrap());

        if (command == hsd::string{"end"})
        {
            break;
        }
        else if (command.starts_with("type "))
        {
            for (hsd::usize index = 5; index < command.length(); ++index)
            {
                buffer.type(command[index]);
            }
        }
        else if (command.starts_with("left "))
        {
            buffer.move_left(parse_count(command, 5));
        }
        else if (command.starts_with("right "))
        {
            buffer.move_right(parse_count(command, 6));
        }
        else if (command == hsd::string{"back"})
        {
            buffer.backspace();
        }
        else if (command == hsd::string{"del"})
        {
            buffer.erase_forward();
        }
        else if (command == hsd::string{"show"})
        {
            hsd::println("[{}] cursor at {}"_fmt, buffer.text(), buffer.cursor());
        }
        else
        {
            hsd::println("unknown command: {}"_fmt, command);
        }
    }

    return 0;
}
