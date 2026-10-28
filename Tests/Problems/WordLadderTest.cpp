#include <cassert>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>

#include <Support/TestSupport.hpp>

static int hsd_word_ladder(
    const char* begin_word,
    const char* end_word,
    const char* const* words,
    std::size_t word_count)
{
    hsd::unordered_map<hsd::string, bool> dictionary;
    for (std::size_t index = 0; index < word_count; ++index)
    {
        hsd::string key{words[index]};
        dictionary.emplace(key, true);
        assert(test_support::to_string(key) == words[index]);
    }

    hsd::string begin{hsd::string_view{begin_word}};
    hsd::string end{hsd::string_view{end_word}};
    if (begin == end)
    {
        return 1;
    }
    if (!dictionary.at(end).is_ok())
    {
        return 0;
    }

    hsd::unordered_map<hsd::string, int> distances;
    hsd::vector<hsd::string> queue;
    queue.push_back(begin);
    distances.emplace(begin, 1);
    assert(test_support::to_string(begin) == begin_word);

    for (std::size_t head = 0; head < queue.size(); ++head)
    {
        const hsd::string current = queue[head];
        const int distance = distances.at(current).unwrap().get();

        for (std::size_t position = 0; position < current.length(); ++position)
        {
            const char original = current[position];
            for (char letter = 'a'; letter <= 'z'; ++letter)
            {
                if (letter == original)
                {
                    continue;
                }
                hsd::string candidate{current};
                candidate[position] = letter;
                if (!dictionary.at(candidate).is_ok() ||
                    distances.at(candidate).is_ok())
                {
                    continue;
                }

                if (candidate == end)
                {
                    return distance + 1;
                }
                queue.push_back(candidate);
                distances.emplace(candidate, distance + 1);
                assert(test_support::to_string(candidate) != "");
            }
        }
    }

    return 0;
}

static int std_word_ladder(
    const std::string& begin_word,
    const std::string& end_word,
    const std::vector<std::string>& words)
{
    std::unordered_set<std::string> dictionary(words.begin(), words.end());
    std::unordered_map<std::string, int> distances{{begin_word, 1}};
    std::queue<std::string> queue;
    queue.push(begin_word);

    while (!queue.empty())
    {
        std::string current = queue.front();
        queue.pop();
        const int distance = distances.at(current);
        for (std::size_t position = 0; position < current.size(); ++position)
        {
            const char original = current[position];
            for (char letter = 'a'; letter <= 'z'; ++letter)
            {
                current[position] = letter;
                if (dictionary.contains(current) && !distances.contains(current))
                {
                    if (current == end_word) return distance + 1;
                    distances.emplace(current, distance + 1);
                    queue.push(current);
                }
            }
            current[position] = original;
        }
    }
    return begin_word == end_word ? 1 : 0;
}

static void check_case(
    const char* begin_word,
    const char* end_word,
    const char* const* words,
    std::size_t word_count)
{
    std::vector<std::string> reference_words;
    for (std::size_t index = 0; index < word_count; ++index)
    {
        reference_words.emplace_back(words[index]);
    }
    assert(hsd_word_ladder(begin_word, end_word, words, word_count) ==
        std_word_ladder(begin_word, end_word, reference_words));
}

int main()
{
    const char* words[] = {"hot", "dot", "dog", "lot", "log", "cog"};
    check_case("hit", "cog", words, 6);
    check_case("hit", "zzz", words, 6);
    check_case("hit", "hit", words, 6);
}