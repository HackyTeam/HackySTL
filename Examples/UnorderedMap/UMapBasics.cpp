// Title: Unordered map basics
// Level: Basic
// Shows: emplace, operator[], checked at(), size and erase on hsd::unordered_map.

#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename Map>
void print_all(const Map& scores)
{
    // The map keeps its records in insertion order, so this output is stable.
    for (const auto& entry : scores)
    {
        hsd::println("  {} -> {}"_fmt, entry.first, entry.second);
    }
}

int main()
{
    hsd::unordered_map<hsd::string, hsd::i32> scores;

    // emplace only inserts when the key is new and reports it through `second`.
    auto first = scores.emplace(hsd::string{"alice"}, 90);
    auto again = scores.emplace(hsd::string{"alice"}, 10);
    hsd::println("first insert: {}, second insert: {}"_fmt,
        first.second ? "inserted" : "kept", again.second ? "inserted" : "kept");

    // operator[] finds the key or inserts a default value for it.
    scores[hsd::string{"bob"}] = 75;
    scores[hsd::string{"carol"}] += 60;
    hsd::println("size: {}"_fmt, scores.size());
    print_all(scores);

    // at() never inserts: it returns a result that fails for a missing key.
    auto found = scores.at(hsd::string{"bob"});
    if (found.is_ok())
    {
        hsd::println("bob has {}"_fmt, found.unwrap().get());
    }

    auto missing = scores.at(hsd::string{"dave"});
    hsd::println("dave present: {}"_fmt, missing.is_ok() ? "true" : "false");

    // erase takes an iterator, so walk to the record first.
    for (auto iter = scores.begin(); iter != scores.end(); ++iter)
    {
        if (iter->first == hsd::string{"bob"})
        {
            scores.erase(iter).unwrap();
            break;
        }
    }

    hsd::println("after erase, size: {}"_fmt, scores.size());
    print_all(scores);

    return 0;
}
