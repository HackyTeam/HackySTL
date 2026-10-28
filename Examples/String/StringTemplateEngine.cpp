// Title: A string template engine
// Level: Advanced
// Shows: hsd::string with unordered_map to fill {{name}} placeholders, with defaults and escapes, from key: value lines on stdin.

#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

using variables = hsd::unordered_map<hsd::string, hsd::string>;

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

hsd::string slice(const hsd::string& text, hsd::usize from, hsd::usize to)
{
    hsd::string result;
    for (hsd::usize i = from; i < to; ++i)
    {
        result.push_back(text[i]);
    }
    return result;
}

hsd::string trim(const hsd::string& text)
{
    hsd::usize first = 0;
    hsd::usize last = text.length();

    while (first < last && text[first] == ' ')
    {
        ++first;
    }
    while (last > first && text[last - 1] == ' ')
    {
        --last;
    }
    return slice(text, first, last);
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

// Reads "key: value" lines. Lines without ':' are counted as rejected.
hsd::usize read_variables(variables& vars)
{
    hsd::usize rejected = 0;

    for_each_line(read_all_input(), [&](const hsd::string& text)
    {
        hsd::usize colon = text.find(':');

        if (colon == hsd::string::npos || colon == 0)
        {
            ++rejected;
            return;
        }

        vars[trim(slice(text, 0, colon))] = trim(slice(text, colon + 1, text.length()));
    });

    return rejected;
}

struct render_report
{
    hsd::usize replaced = 0;
    hsd::usize missing = 0;
};

// Replaces {{name}} with its value and {{name|fallback}} with the fallback when
// the name is unknown. "\{{" prints a literal "{{". Unknown names without a
// fallback are left visible as <name?> so the mistake shows up in the output.
hsd::string render(const hsd::string& tmpl, const variables& vars, render_report& report)
{
    hsd::string result;
    hsd::usize pos = 0;

    while (pos < tmpl.length())
    {
        hsd::usize open = tmpl.find("{{", pos);
        if (open == hsd::string::npos)
        {
            append(result, slice(tmpl, pos, tmpl.length()));
            break;
        }

        if (open > pos && tmpl[open - 1] == '\\')
        {
            // Keep the text before the backslash, then emit the braces as they are.
            append(result, slice(tmpl, pos, open - 1));
            append(result, "{{");
            pos = open + 2;
            continue;
        }

        append(result, slice(tmpl, pos, open));

        hsd::usize close = tmpl.find("}}", open + 2);
        if (close == hsd::string::npos)
        {
            // An unclosed placeholder is copied through unchanged.
            append(result, slice(tmpl, open, tmpl.length()));
            break;
        }

        hsd::string inner = trim(slice(tmpl, open + 2, close));
        hsd::string name = inner;
        hsd::string fallback;
        bool has_fallback = false;

        hsd::usize bar = inner.find('|');
        if (bar != hsd::string::npos)
        {
            name = trim(slice(inner, 0, bar));
            fallback = trim(slice(inner, bar + 1, inner.length()));
            has_fallback = true;
        }

        auto found = vars.at(name);
        if (found.is_ok())
        {
            append(result, found.unwrap().get());
            ++report.replaced;
        }
        else if (has_fallback)
        {
            append(result, fallback);
            ++report.replaced;
        }
        else
        {
            append(result, "<");
            append(result, name);
            append(result, "?>");
            ++report.missing;
        }

        pos = close + 2;
    }

    return result;
}

int main()
{
    variables vars;
    hsd::usize rejected = read_variables(vars);
    hsd::println("variables: {}, rejected lines: {}"_fmt, vars.size(), rejected);

    const char* templates[] = {
        "Dear {{ name }},",
        "Your order {{order}} ships to {{city|an unknown city}}.",
        "Total: {{total}} {{currency|EUR}}",
        "Write \\{{name}} to print a placeholder, or {{name",
    };

    render_report report;
    for (const char* source : templates)
    {
        hsd::println("{}"_fmt, render(hsd::string{source}, vars, report));
    }

    hsd::println("totals: replaced {} missing {}"_fmt, report.replaced, report.missing);

    return 0;
}
