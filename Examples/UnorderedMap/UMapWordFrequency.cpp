// Title: Word frequency
// Level: Intermediate
// Shows: counting words from stdin with an unordered_map and ranking them.

#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Base/Algorithm.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

struct word_count
{
    hsd::string word;
    hsd::i32 count;

    // quick_sort only requires operator> on the elements, even with a predicate.
    bool operator>(const word_count& rhs) const
    {
        return count > rhs.count;
    }
};

static char lower_letter(char value)
{
    return (value >= 'A' && value <= 'Z') ? static_cast<char>(value - 'A' + 'a') : value;
}

static bool is_letter(char value)
{
    return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
}

int main()
{
    hsd::unordered_map<hsd::string, hsd::i32> counts;
    hsd::i32 total = 0;

    // read_line fails once stdin is exhausted, which ends the loop. The stream
    // hands over a line only when more input follows it, so the input finishes
    // with an "end" line that also stops the loop.
    for (auto line = hsd::io::cin().read_line(); line.is_ok();
        line = hsd::io::cin().read_line())
    {
        hsd::string text = hsd::move(line.unwrap());

        if (text == hsd::string{"end"})
        {
            break;
        }

        hsd::string word;

        // The string type has no split, so cut words by hand at non-letters.
        for (hsd::usize index = 0; index <= text.length(); ++index)
        {
            const char current = index < text.length() ? text[index] : ' ';

            if (is_letter(current))
            {
                word.push_back(lower_letter(current));
            }
            else if (word.length() != 0)
            {
                counts[word] += 1;
                ++total;
                word.clear();
            }
        }
    }

    hsd::println("words: {} distinct: {}"_fmt, total, counts.size());

    hsd::vector<word_count> ranking;
    for (const auto& entry : counts)
    {
        ranking.push_back(word_count{entry.first, entry.second});
    }

    // Higher count first. A predicate returning true places `lhs` after `rhs`,
    // and equal counts fall back to alphabetical order so ties are stable.
    hsd::quick_sort(ranking, [](const word_count& lhs, const word_count& rhs)
    {
        if (lhs.count != rhs.count)
        {
            return lhs.count < rhs.count;
        }
        return lhs.word > rhs.word;
    });

    for (hsd::usize index = 0; index < ranking.size() && index < 3; ++index)
    {
        hsd::println("{} {}"_fmt, ranking[index].word, ranking[index].count);
    }

    return 0;
}
