// Title: Editing strings
// Level: Intermediate
// Shows: trim, collapse spaces, replace and reverse written on hsd::string, applied to lines read from stdin.

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// hsd::string has no trim, replace or reverse, so they are written here
// from the pieces it does have: indexing, push_back and erase_for.

// operator+= panics on an empty operand, and its const char* form does not
// update the length when spare capacity exists, so text is appended one
// character at a time with push_back.
void append(hsd::string& target, const hsd::string& text)
{
    for (hsd::usize i = 0; i < text.length(); ++i)
    {
        target.push_back(text[i]);
    }
}

void append(hsd::string& target, const char* text)
{
    for (; *text != '\0'; ++text)
    {
        target.push_back(*text);
    }
}

bool is_blank(char letter)
{
    return letter == ' ' || letter == '\t';
}

hsd::string trim(const hsd::string& text)
{
    hsd::usize first = 0;
    hsd::usize last = text.length();

    while (first < last && is_blank(text[first]))
    {
        ++first;
    }

    while (last > first && is_blank(text[last - 1]))
    {
        --last;
    }

    hsd::string result;
    for (hsd::usize i = first; i < last; ++i)
    {
        result.push_back(text[i]);
    }
    return result;
}

// Turns every run of blanks into one space.
hsd::string collapse_spaces(const hsd::string& text)
{
    hsd::string result;
    bool in_blank = false;

    for (hsd::usize i = 0; i < text.length(); ++i)
    {
        if (is_blank(text[i]))
        {
            if (!in_blank)
            {
                result.push_back(' ');
            }
            in_blank = true;
        }
        else
        {
            result.push_back(text[i]);
            in_blank = false;
        }
    }
    return result;
}

hsd::string replace_all(const hsd::string& text, const char* from, const char* to)
{
    hsd::string needle{from};
    hsd::string result;
    hsd::usize pos = 0;

    while (needle.length() != 0)
    {
        hsd::usize hit = text.find(from, pos);
        if (hit == hsd::string::npos)
        {
            break;
        }

        for (hsd::usize i = pos; i < hit; ++i)
        {
            result.push_back(text[i]);
        }
        append(result, to);
        pos = hit + needle.length();
    }

    for (hsd::usize i = pos; i < text.length(); ++i)
    {
        result.push_back(text[i]);
    }
    return result;
}

hsd::string reversed(const hsd::string& text)
{
    hsd::string result;
    for (hsd::usize i = text.length(); i > 0; --i)
    {
        result.push_back(text[i - 1]);
    }
    return result;
}

hsd::string upper_case(hsd::string text)
{
    for (hsd::usize i = 0; i < text.length(); ++i)
    {
        if (text[i] >= 'a' && text[i] <= 'z')
        {
            text[i] = static_cast<char>(text[i] - 'a' + 'A');
        }
    }
    return text;
}

// Reads all of stdin one line at a time with read_line(). Every line, including
// blank lines and a final line without a newline, is kept, and each one
// ends with '\n' so the text can be split on newlines.
hsd::string read_all_input()
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

// Calls the visitor with every non-empty line of the text.
template <typename Visitor>
void for_each_line(const hsd::string& text, Visitor visitor)
{
    hsd::usize pos = 0;

    while (pos < text.length())
    {
        hsd::usize end = text.find('\n', pos);
        if (end == hsd::string::npos)
        {
            end = text.length();
        }

        if (end > pos)
        {
            hsd::string line;
            for (hsd::usize i = pos; i < end; ++i)
            {
                line.push_back(text[i]);
            }
            visitor(line);
        }
        pos = end + 1;
    }
}

int main()
{
    hsd::usize line_number = 0;

    for_each_line(read_all_input(), [&](const hsd::string& text)
    {
        ++line_number;

        hsd::string tidy = collapse_spaces(trim(text));
        hsd::println("line {}: '{}'"_fmt, line_number, tidy);
        hsd::println("  replaced: '{}'"_fmt, replace_all(tidy, "cat", "dog"));
        hsd::println("  reversed: '{}'"_fmt, reversed(tidy));
        hsd::println("  upper:    '{}'"_fmt, upper_case(tidy));
    });

    // erase_for() removes a range of characters in place.
    hsd::string edited{"keep [remove this] keep"};
    hsd::usize open = edited.find('[');
    hsd::usize close = edited.find(']');
    if (open != hsd::string::npos && close != hsd::string::npos)
    {
        edited.erase_for(edited.begin() + open, edited.begin() + close + 2).unwrap();
    }
    hsd::println("erase_for: '{}'"_fmt, edited);
    hsd::println("lines processed: {}"_fmt, line_number);

    return 0;
}
