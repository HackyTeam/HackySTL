// Title: Pair structured bindings
// Level: Intermediate
// Shows: hsd::pair with structured bindings, pairs in a vector, and why tuple needs get<Index>() instead.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Pair.hpp>
#include <Wrappers/Tuple.hpp>

using namespace hsd::format_literals;

// Returns the smallest and largest value as a pair.
static hsd::pair<hsd::i32, hsd::i32> min_max(const hsd::vector<hsd::i32>& values)
{
    hsd::i32 smallest = values[0];
    hsd::i32 largest = values[0];

    for (hsd::usize index = 1; index < values.size(); ++index)
    {
        if (values[index] < smallest)
        {
            smallest = values[index];
        }

        if (values[index] > largest)
        {
            largest = values[index];
        }
    }

    return hsd::pair{smallest, largest};
}

int main()
{
    hsd::vector<hsd::i32> values{{4, -2, 9, 0, 7}};

    // A pair has public members `first` and `second`, so structured bindings work.
    auto [low, high] = min_max(values);
    hsd::println("min: {} max: {}"_fmt, low, high);

    // Bindings by reference modify the pair in place.
    hsd::pair<hsd::i32, hsd::i32> range = min_max(values);
    auto& [range_low, range_high] = range;
    range_low -= 1;
    range_high += 1;
    hsd::println("widened: {} to {}"_fmt, range.first, range.second);

    // make_pair deduces the element types.
    auto made = hsd::make_pair(1, 'a');
    auto [number, letter] = made;
    hsd::println("made: {} {}"_fmt, number, letter);

    // A vector of pairs makes a small table that is easy to walk with bindings.
    hsd::vector<hsd::pair<const char*, hsd::i32>> scores;
    scores.push_back(hsd::pair<const char*, hsd::i32>{"ada", 90});
    scores.push_back(hsd::pair<const char*, hsd::i32>{"bob", 72});
    scores.push_back(hsd::pair<const char*, hsd::i32>{"cy", 85});

    hsd::i32 sum = 0;
    for (const auto& [name, score] : scores)
    {
        hsd::println("{}: {}"_fmt, name, score);
        sum += score;
    }
    hsd::println("sum: {}"_fmt, sum);

    // A tuple has no such support. Read its elements with get<Index>() instead.
    hsd::tuple triple = {low, high, "range"};
    hsd::println("tuple: {} {} {}"_fmt, triple.get<0>(), triple.get<1>(), triple.get<2>());
    // Pairs compare element by element.
    hsd::pair<hsd::i32, hsd::string> left{1, hsd::string{"one"}};
    hsd::pair<hsd::i32, hsd::string> right{1, hsd::string{"one"}};
    hsd::pair<hsd::i32, hsd::string> other{2, hsd::string{"one"}};
    hsd::println("pairs equal: {}, different: {}"_fmt,
        left == right ? "true" : "false", left == other ? "true" : "false");
    return 0;
}
