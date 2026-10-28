// Title: Tokenizing with views
// Level: Intermediate
// Shows: splitting text into tokens with string_view and find_first_of, with no copies, over lines read from stdin.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::string_view_literals;
using namespace hsd::format_literals;

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

// sub_string() is an error when the start is at or past the end, so an empty
// range is handled here instead of being passed on.
hsd::string_view slice(hsd::string_view text, hsd::usize from, hsd::usize to)
{
    if (from >= to || from >= text.length())
    {
        return hsd::string_view{};
    }

    return text.sub_string(from, to - from).unwrap();
}

// Splits `text` at any character found in `separators`. Empty tokens are skipped.
// Every token is a view into `text`, so `text` must outlive the vector.
hsd::vector<hsd::string_view> tokenize(hsd::string_view text, hsd::string_view separators)
{
    hsd::vector<hsd::string_view> tokens;
    hsd::string_view rest = text;

    while (rest.length() != 0)
    {
        hsd::usize cut = rest.find_first_of(separators);
        hsd::usize end = cut == hsd::string_view::npos ? rest.length() : cut;

        if (end > 0)
        {
            tokens.push_back(slice(rest, 0, end));
        }

        // Step over the separator itself, or finish if there was none.
        rest = cut == hsd::string_view::npos ? hsd::string_view{} : slice(rest, cut + 1, rest.length());
    }

    return tokens;
}

int main()
{
    hsd::string input = read_all_input();
    hsd::string_view text{input.c_str(), input.length()};

    // Each line is itself a view of the whole input.
    hsd::usize line_number = 0;
    hsd::usize total_tokens = 0;
    hsd::string_view longest;

    hsd::string_view rest = text;
    while (rest.length() != 0)
    {
        hsd::usize newline = rest.find('\n');
        hsd::string_view line = slice(rest, 0, newline == hsd::string_view::npos ? rest.length() : newline);
        rest = newline == hsd::string_view::npos ? hsd::string_view{} : slice(rest, newline + 1, rest.length());

        ++line_number;
        auto tokens = tokenize(line, " ,;:"_sv);
        hsd::print("line {} has {} tokens:"_fmt, line_number, tokens.size());

        for (const auto& token : tokens)
        {
            hsd::print(" [{}]"_fmt, token);
            if (token.length() > longest.length())
            {
                longest = token;
            }
        }
        hsd::println(""_fmt);
        total_tokens += tokens.size();
    }

    hsd::println("total tokens: {}"_fmt, total_tokens);

    // Printing an empty view crashes, so the empty case is handled separately.
    if (longest.length() == 0)
    {
        hsd::println("longest token: none"_fmt);
    }
    else
    {
        hsd::println("longest token: {} ({} characters)"_fmt, longest, longest.length());
    }

    return 0;
}
