// Title: Switch basics
// Level: Basic
// Shows: switch_cases with equal_case on numbers and strings, and the default_case fallback.

#include <Base/Switch.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

static void describe_number(hsd::i32 value)
{
    // The cases are tried in order and the first match runs its function.
    // C++ switch cannot do this on strings, and the default case is required.
    hsd::switch_cases(
        value,
        hsd::equal_case{1, [] { hsd::println("one"_fmt); }},
        hsd::equal_case{2, [] { hsd::println("two"_fmt); }},
        hsd::equal_case{3, [] { hsd::println("three"_fmt); }},
        hsd::default_case{[&] { hsd::println("{} is not 1, 2 or 3"_fmt, value); }}
    );
}

static void greet(hsd::string_view language)
{
    hsd::switch_cases(
        language,
        hsd::equal_case{"en", [] { hsd::println("Hello"_fmt); }},
        hsd::equal_case{"fr", [] { hsd::println("Bonjour"_fmt); }},
        hsd::equal_case{"ro", [] { hsd::println("Salut"_fmt); }},
        hsd::default_case{[] { hsd::println("..."_fmt); }}
    );
}

int main()
{
    describe_number(2);
    describe_number(3);
    describe_number(42);

    greet("fr"_sv);
    greet("ro"_sv);
    greet("xx"_sv);

    // The cases can also fill a variable instead of printing.
    hsd::i32 selected = 0;
    hsd::switch_cases(
        "beta"_sv,
        hsd::equal_case{"alpha", [&] { selected = 1; }},
        hsd::equal_case{"beta", [&] { selected = 2; }},
        hsd::default_case{[&] { selected = -1; }}
    );
    hsd::println("chosen: {}"_fmt, selected);

    return 0;
}
