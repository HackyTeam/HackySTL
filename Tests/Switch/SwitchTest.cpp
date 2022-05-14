#include <Io.hpp>
#include <StringView.hpp>

int main()
{
    using namespace hsd::format_literals;
    using namespace hsd::string_view_literals;
    
    hsd::switch_cases("Test value"_sv, 
        hsd::EqualCase{"test",       []{hsd::println("1"_fmt);}},
        hsd::EqualCase{"value",      []{hsd::println("2"_fmt);}},
        hsd::EqualCase{"Test",       []{hsd::println("3"_fmt);}},
        hsd::EqualCase{"Value",      []{hsd::println("4"_fmt);}},
        hsd::EqualCase{"test value", []{hsd::println("5"_fmt);}},
        hsd::EqualCase{"test Value", []{hsd::println("6"_fmt);}},
        hsd::EqualCase{
            "Test value", []() -> hsd::option_err<hsd::runtime_error>
            {
                hsd::println("7"_fmt);
                return {};
                //return hsd::runtime_error{"Got the 7th value"};
            }
        },
        hsd::EqualCase{"Test Value", []{hsd::println("8"_fmt);}},
        hsd::DefaultCase{[]{hsd::print("Default"_fmt);}}
    );

    hsd::switch_cases(
        -234,
        hsd::LessCase   {120, []{hsd::println("Less than 120"_fmt   );}},
        hsd::EqualCase  {120, []{hsd::println("Equal than 120"_fmt  );}},
        hsd::GreaterCase{120, []{hsd::println("Greater than 120"_fmt);}},
        hsd::DefaultCase{
            []() -> hsd::option_err<hsd::runtime_error>
            {
                return hsd::runtime_error{"It's impossible to get here"};
            }
        }
    );

    hsd::switch_cases("Test value"_sv, 
        hsd::LessCase   {"Tf", []{hsd::println("Less than Tf\n"_fmt   );}},
        hsd::EqualCase  {"Tf", []{hsd::println("Equal than Tf\n"_fmt  );}},
        hsd::GreaterCase{"Tf", []{hsd::println("Greater than Tf\n"_fmt);}},
        hsd::DefaultCase{[]{hsd::println_err("HOW"_fmt);}}
    );
}