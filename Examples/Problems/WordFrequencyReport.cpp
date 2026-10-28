// Title: Word frequency report
// Level: Intermediate
// Shows: tokenizing a text, counting in an unordered_map and ranking with quick_sort.

#include <Base/Algorithm.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

struct word_count
{
    hsd::string word;
    hsd::usize count;

    // quick_sort only accepts element types that have operator>, even when a
    // predicate is passed. The predicate below decides the real order.
    bool operator>(const word_count& other) const
    {
        return count > other.count;
    }
};

static bool is_letter(char character)
{
    return (character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z');
}

// Splits the text into lowercase words, dropping punctuation and spaces.
static hsd::vector<hsd::string> tokenize(hsd::string_view text)
{
    hsd::vector<hsd::string> words;
    hsd::string current;

    for (const char character : text)
    {
        if (is_letter(character))
        {
            current.push_back(character >= 'A' && character <= 'Z'
                ? static_cast<char>(character - 'A' + 'a') : character);
        }
        else if (current.length() != 0)
        {
            words.push_back(hsd::move(current));
            current.clear();
        }
    }

    if (current.length() != 0)
    {
        words.push_back(hsd::move(current));
    }
    return words;
}

int main()
{
    constexpr auto text =
        "The quick brown fox jumps over the lazy dog. The dog sleeps; "
        "the fox, quick as ever, jumps again. A lazy day for a lazy dog!"_sv;

    auto words = tokenize(text);

    // The map keeps one counter per distinct word.
    hsd::unordered_map<hsd::string, hsd::usize> counts;
    for (const auto& word : words)
    {
        ++counts[word];
    }

    hsd::vector<word_count> ranking;
    for (const auto& entry : counts)
    {
        ranking.push_back(word_count{entry.first, entry.second});
    }

    // The predicate returns true when `left` belongs after `right`:
    // fewer occurrences go later, and ties are broken alphabetically.
    hsd::quick_sort(ranking, [](const word_count& left, const word_count& right)
    {
        if (left.count != right.count)
        {
            return left.count < right.count;
        }
        return right.word < left.word;
    });

    hsd::println("total words: {}"_fmt, words.size());
    hsd::println("distinct words: {}"_fmt, ranking.size());
    hsd::println("top words:"_fmt);

    const hsd::usize shown = ranking.size() < 5 ? ranking.size() : 5;
    for (hsd::usize index = 0; index < shown; ++index)
    {
        hsd::println("{}. {} x{}"_fmt,
            index + 1, ranking[index].word, ranking[index].count);
    }

    hsd::usize longest = 0;
    for (const auto& entry : ranking)
    {
        if (entry.word.length() > longest)
        {
            longest = entry.word.length();
        }
    }
    hsd::println("longest word length: {}"_fmt, longest);
}
