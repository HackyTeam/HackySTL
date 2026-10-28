// Title: Parallel word count
// Level: Advanced
// Shows: threads counting chunks into private slots, an atomic total and a merge on the main thread.

#include <Base/Algorithm.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Atomic.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

using word_counts = hsd::unordered_map<hsd::string, hsd::usize>;

struct ranked_word
{
    hsd::string word;
    hsd::usize count;

    // quick_sort needs operator> on the element type; the predicate sets the order.
    bool operator>(const ranked_word& other) const
    {
        return count > other.count;
    }
};

// Counts lowercase words in one chunk. Each worker only touches its own map.
static void count_words(hsd::string_view text, word_counts& counts, hsd::atomic_usize& total)
{
    hsd::string current;
    hsd::usize local_total = 0;

    auto finish_word = [&]
    {
        if (current.length() != 0)
        {
            ++counts[current];
            ++local_total;
            current.clear();
        }
    };

    for (const char character : text)
    {
        if (character >= 'a' && character <= 'z')
        {
            current.push_back(character);
        }
        else
        {
            finish_word();
        }
    }
    finish_word();

    total.fetch_add(local_total);
}

int main()
{
    const hsd::string_view chunks[] = {
        "the cat sat on the mat while the dog slept"_sv,
        "a cat and a dog met on the road to the town"_sv,
        "the town slept and the cat watched the road"_sv,
        "dog and cat and mat all in the same town"_sv
    };
    constexpr hsd::usize chunk_count = sizeof(chunks) / sizeof(chunks[0]);

    // One pre-sized slot per worker: no two threads share a map.
    hsd::vector<word_counts> slots;
    for (hsd::usize index = 0; index < chunk_count; ++index)
    {
        slots.emplace_back();
    }

    hsd::atomic_usize total{0};
    hsd::vector<hsd::thread> workers;
    for (hsd::usize index = 0; index < chunk_count; ++index)
    {
        workers.emplace_back([&, index]
        {
            count_words(chunks[index], slots[index], total);
        });
    }

    // Joining in order makes the rest of the program independent of scheduling.
    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    word_counts merged;
    for (hsd::usize index = 0; index < chunk_count; ++index)
    {
        hsd::usize chunk_total = 0;
        for (const auto& entry : slots[index])
        {
            chunk_total += entry.second;
            merged[entry.first] += entry.second;
        }
        hsd::println("chunk {}: {} words, {} distinct"_fmt,
            index, chunk_total, slots[index].size());
    }

    hsd::println("total words: {}"_fmt, total.load());
    hsd::println("distinct words: {}"_fmt, merged.size());

    hsd::vector<ranked_word> ranking;
    for (const auto& entry : merged)
    {
        ranking.push_back(ranked_word{entry.first, entry.second});
    }
    hsd::quick_sort(ranking, [](const ranked_word& left, const ranked_word& right)
    {
        if (left.count != right.count)
        {
            return left.count < right.count;
        }
        return right.word < left.word;
    });

    hsd::println("most common:"_fmt);
    for (hsd::usize index = 0; index < 4; ++index)
    {
        hsd::println("  {} x{}"_fmt, ranking[index].word, ranking[index].count);
    }
}
