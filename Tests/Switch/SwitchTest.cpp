#include <Extra/Swtich.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::string_view_literals;
    hsd::switch_cases("Test value"_sv, 
        hsd::EqualCase{"test",       []{hsd::io::print<"1\n">();}},
        hsd::EqualCase{"value",      []{hsd::io::print<"2\n">();}},
        hsd::EqualCase{"Test",       []{hsd::io::print<"3\n">();}},
        hsd::EqualCase{"Value",      []{hsd::io::print<"4\n">();}},
        hsd::EqualCase{"test value", []{hsd::io::print<"5\n">();}},
        hsd::EqualCase{"test Value", []{hsd::io::print<"6\n">();}},
        hsd::EqualCase{
            "Test value", []() -> hsd::Result<void, hsd::runtime_error>
            {
                hsd::io::print<"7\n">();
                return {};
                // return hsd::runtime_error{"Got the 7th value"};
            }
        },
        hsd::EqualCase{"Test Value", []{hsd::io::print<"8\n">();}},
        hsd::DefaultCase{[]{hsd::io::print<"Default\n">();}}
    );

    hsd::switch_cases(
        -234,
        hsd::LessCase   {120, []{hsd::io::print<"Less than 120\n">();   }},
        hsd::EqualCase  {120, []{hsd::io::print<"Equal than 120\n">();  }},
        hsd::GreaterCase{120, []{hsd::io::print<"Greater than 120\n">();}},
        hsd::DefaultCase{
            []() -> hsd::Result<void, hsd::runtime_error>
            {
                return hsd::runtime_error{"It's impossible to get here"};
            }
        }
    );

    hsd::switch_cases("Test value"_sv, 
        hsd::LessCase   {"Tf", []{hsd::io::print<"Less than Tf\n">();   }},
        hsd::EqualCase  {"Tf", []{hsd::io::print<"Equal than Tf\n">();  }},
        hsd::GreaterCase{"Tf", []{hsd::io::print<"Greater than Tf\n">();}},
        hsd::DefaultCase{[]{hsd::io::print<"HOW\n">();}}
    );
}