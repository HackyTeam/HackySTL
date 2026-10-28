// Title: Switch ranges
// Level: Intermediate
// Shows: less_case and greater_case for banding values, and why the order of cases matters.

#include <Base/Switch.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Every case compares the switched value with its own bound, and the first
// one that holds wins. Putting the lowest bound first turns a chain of less
// cases into ranges.
static const char* describe_temperature(hsd::i32 celsius)
{
    const char* label = "";

    hsd::switch_cases(
        celsius,
        hsd::less_case{0, [&] { label = "freezing"; }},
        hsd::less_case{15, [&] { label = "cold"; }},
        hsd::less_case{25, [&] { label = "mild"; }},
        hsd::less_case{35, [&] { label = "warm"; }},
        hsd::default_case{[&] { label = "hot"; }}
    );

    return label;
}

// less, equal and greater cases can be mixed against the same bound.
static const char* compare_with_target(hsd::i32 guess, hsd::i32 target)
{
    const char* label = "";

    hsd::switch_cases(
        guess,
        hsd::less_case{target, [&] { label = "too low"; }},
        hsd::greater_case{target, [&] { label = "too high"; }},
        hsd::equal_case{target, [&] { label = "correct"; }},
        hsd::default_case{[&] { label = "unreachable"; }}
    );

    return label;
}

int main()
{
    const hsd::i32 readings[]{-10, 0, 14, 15, 24, 30, 35, 41};

    for (hsd::i32 reading : readings)
    {
        hsd::println("{} C is {}"_fmt, reading, describe_temperature(reading));
    }

    // Greater cases work from the other end: the biggest bound goes first.
    hsd::i32 stars = 0;
    hsd::i32 score = 87;
    hsd::switch_cases(
        score,
        hsd::greater_case{90, [&] { stars = 3; }},
        hsd::greater_case{75, [&] { stars = 2; }},
        hsd::greater_case{50, [&] { stars = 1; }},
        hsd::default_case{[&] { stars = 0; }}
    );
    hsd::println("score {} earns {} stars"_fmt, score, stars);

    const hsd::i32 guesses[]{3, 8, 5};
    for (hsd::i32 guess : guesses)
    {
        hsd::println("guess {}: {}"_fmt, guess, compare_with_target(guess, 5));
    }

    return 0;
}
