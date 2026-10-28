// Title: Text processing with views
// Level: Advanced
// Shows: trimming, word splitting and a palindrome test built only from range views on stdin lines.

#include <Containers/Range.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// A string converts to a view explicitly; views are what the adaptors work on.
static hsd::string_view view_of(const hsd::string& text)
{
    return static_cast<hsd::string_view>(text);
}

static bool is_space(char value)
{
    return hsd::cstring::iswhitespace(value);
}

static bool is_not_space(char value)
{
    return is_space(value) == false;
}

static bool is_letter(char value)
{
    return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
}

static char lowered(char value)
{
    return (value >= 'A' && value <= 'Z') ? static_cast<char>(value - 'A' + 'a') : value;
}

// Removes blanks at both ends: drop the leading ones, flip the view around,
// drop the (former) trailing ones, and flip it back while copying out.
static hsd::string trim(hsd::string_view text)
{
    hsd::string result;
    auto view = text
        | hsd::views::drop_while(is_space)
        | hsd::views::reverse
        | hsd::views::drop_while(is_space);

    for (const char value : view | hsd::views::reverse)
    {
        result.push_back(value);
    }

    return result;
}

struct word_stats
{
    hsd::usize count = 0;
    hsd::usize longest = 0;
    hsd::string longest_word;
};

// Walks the line word by word. drop_while skips the blanks, take_while cuts one
// word, and drop moves past it. No index arithmetic on the text is needed.
static word_stats analyze(const hsd::string& line)
{
    word_stats stats;
    auto rest = view_of(line) | hsd::views::drop(0);

    while (true)
    {
        auto start = rest | hsd::views::drop_while(is_space);

        if (start.size() == 0)
        {
            break;
        }

        auto word = start | hsd::views::take_while(is_not_space);
        ++stats.count;

        if (word.size() > stats.longest)
        {
            stats.longest = word.size();
            stats.longest_word.clear();
            for (const char value : word)
            {
                stats.longest_word.push_back(value);
            }
        }

        rest = start | hsd::views::drop(word.size());
    }

    return stats;
}

// Keeps only letters in lower case, then compares the text with its reverse.
static bool is_palindrome(const hsd::string& line)
{
    hsd::vector<char> letters;

    for (const char value : view_of(line)
        | hsd::ranges::filter(is_letter)
        | hsd::ranges::transform(lowered))
    {
        letters.push_back(value);
    }

    // Reversing an empty vector steps before its start, so rule that out first.
    if (letters.size() == 0)
    {
        return false;
    }

    hsd::usize index = 0;
    for (const char value : letters | hsd::views::reverse)
    {
        if (value != letters[index])
        {
            return false;
        }
        ++index;
    }

    return true;
}

int main()
{
    // The stream hands over a line once more input follows it, so the input
    // finishes with an "end" line that also stops the loop. Lines keep their
    // blanks, which is why trim has something to do.
    for (auto line = hsd::io::cin().read_line(); line.is_ok();
        line = hsd::io::cin().read_line())
    {
        const hsd::string raw = hsd::move(line.unwrap());

        if (raw == hsd::string{"end"})
        {
            break;
        }

        const hsd::string text = trim(view_of(raw));
        const word_stats stats = analyze(text);

        hsd::println("[{}]"_fmt, text);
        hsd::println("  {} words, longest {} (\"{}\"), palindrome: {}"_fmt,
            stats.count, stats.longest, stats.longest_word,
            is_palindrome(text) ? "yes" : "no");
    }

    // Reverse views work on a string as well as on a vector.
    hsd::string edge_word{"xyz"};
    hsd::print("string reversed:"_fmt);
    for (const char letter : edge_word | hsd::views::reverse)
    {
        hsd::print(" {}"_fmt, letter);
    }
    hsd::println(""_fmt);
    return 0;
}
