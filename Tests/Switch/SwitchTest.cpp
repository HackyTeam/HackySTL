#include <Io.hpp>
#include <StringView.hpp>

int main()
{
    using namespace hsd::string_view_literals;
    hsd::switch_cases("Test value"_sv, 
        hsd::EqualCase{"test",       []{hsd_println("1");}},
        hsd::EqualCase{"value",      []{hsd_println("2");}},
        hsd::EqualCase{"Test",       []{hsd_println("3");}},
        hsd::EqualCase{"Value",      []{hsd_println("4");}},
        hsd::EqualCase{"test value", []{hsd_println("5");}},
        hsd::EqualCase{"test Value", []{hsd_println("6");}},
        hsd::EqualCase{
            "Test value", []() -> hsd::Result<void, hsd::runtime_error>
            {
                hsd_println("7");
                return {};
                // return hsd::runtime_error{"Got the 7th value"};
            }
        },
        hsd::EqualCase{"Test Value", []{hsd_println("8");}},
        hsd::DefaultCase{[]{hsd_print("Default");}}
    );

    hsd::switch_cases(
        -234,
        hsd::LessCase   {120, []{hsd_println("Less than 120"   );}},
        hsd::EqualCase  {120, []{hsd_println("Equal than 120"  );}},
        hsd::GreaterCase{120, []{hsd_println("Greater than 120");}},
        hsd::DefaultCase{
            []() -> hsd::Result<void, hsd::runtime_error>
            {
                return hsd::runtime_error{"It's impossible to get here"};
            }
        }
    );

    hsd::switch_cases("Test value"_sv, 
        hsd::LessCase   {"Tf", []{hsd_println("Less than Tf\n"   );}},
        hsd::EqualCase  {"Tf", []{hsd_println("Equal than Tf\n"  );}},
        hsd::GreaterCase{"Tf", []{hsd_println("Greater than Tf\n");}},
        hsd::DefaultCase{[]{hsd_println_err("HOW");}}
    );
}