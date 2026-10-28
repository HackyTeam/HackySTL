#include <cassert>

#include <Base/Switch.hpp>
#include <Containers/StringView.hpp>

int main()
{
    using namespace hsd::string_view_literals;

    int selected = 0;
    hsd::switch_cases(
        "Test value"_sv,
        hsd::equal_case{"test", [&] { selected = 1; }},
        hsd::equal_case{"Test", [&] { selected = 2; }},
        hsd::equal_case{
            "Test value",
            [&]() -> hsd::option_err<hsd::runtime_error>
            {
                selected = 3;
                return {};
            }
        },
        hsd::default_case{[&] { selected = -1; }}
    );
    assert(selected == 3);

    selected = 0;
    hsd::switch_cases(
        7,
        hsd::equal_case{1, [&] { selected = 1; }},
        hsd::equal_case{2, [&] { selected = 2; }},
        hsd::default_case{[&] { selected = -1; }}
    );
    assert(selected == -1);

    selected = 0;
    hsd::switch_cases(
        -234,
        hsd::less_case{120, [&] { selected = 1; }},
        hsd::equal_case{120, [&] { selected = 2; }},
        hsd::greater_case{120, [&] { selected = 3; }},
        hsd::default_case{[&] { selected = -1; }}
    );
    assert(selected == 1);

    selected = 0;
    hsd::switch_cases(
        "Tz"_sv,
        hsd::less_case{"Tf", [&] { selected = 1; }},
        hsd::equal_case{"Tf", [&] { selected = 2; }},
        hsd::greater_case{"Tf", [&] { selected = 3; }},
        hsd::default_case{[&] { selected = -1; }}
    );
    assert(selected == 3);
}
