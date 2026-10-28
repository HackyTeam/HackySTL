#include <cassert>

#include <Serialize/ArgParse.hpp>

using namespace hsd::string_view_literals;

// Regression tests for argument dispatch: a missing trailing value is not read
// past argc, unknown options and unregistered help flags return instead of
// aborting, and a registered help flag runs its callback.
int main()
{
    // The argument array is exactly as long as argc, so reading past it is caught.
    {
        hsd::argument_parser parser = "usage"_sv;
        int calls = 0;
        parser.add("-pair", 2, [&calls](hsd::parser_stream&) { ++calls; }, "-pair a b");

        const char* argv[] = {"program", "-pair", "1"};
        parser.parse(3, argv);
        assert(calls == 0);
    }

    // Values are passed to the callback in order.
    {
        hsd::argument_parser parser = "usage"_sv;
        int first = 0;
        int second = 0;
        parser.add("-pair", 2, [&first, &second](hsd::parser_stream& stream)
        {
            stream.set_data(first, second).unwrap();
        }, "-pair a b");

        const char* argv[] = {"program", "-pair", "7", "9"};
        parser.parse(4, argv);
        assert(first == 7 && second == 9);
    }

    // An unknown option ends parsing without aborting.
    {
        hsd::argument_parser parser = "usage"_sv;
        int calls = 0;
        parser.add("-known", 0, [&calls](hsd::parser_stream&) { ++calls; }, "-known");

        const char* argv[] = {"program", "-unknown"};
        parser.parse(2, argv);
        assert(calls == 0);
    }

    // A help flag without a registered option prints the usage and returns.
    {
        hsd::argument_parser parser = "usage"_sv;
        const char* argv[] = {"program", "-h"};
        parser.parse(2, argv);
    }

    // A registered help flag runs its callback instead of the built-in help.
    {
        hsd::argument_parser parser = "usage"_sv;
        int calls = 0;
        parser.add("-h", 0, [&calls](hsd::parser_stream&) { ++calls; }, "-h");

        const char* argv[] = {"program", "-h"};
        parser.parse(2, argv);
        assert(calls == 1);
    }
}
