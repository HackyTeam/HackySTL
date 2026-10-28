// Title: Command-line tool
// Level: Advanced
// Shows: a small CLI built on argument_parser, with validation done before parse().

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/ArgParse.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

namespace
{
    struct option_info
    {
        const char* name;
        hsd::usize token_count;
        const char* help;
    };

    // One table drives both the registration and the pre-check below.
    constexpr option_info options[] = {
        {"-name", 1, "-name WORD    who to greet (default: world)"},
        {"-times", 1, "-times N      how many lines (default: 1)"},
        {"-scale", 1, "-scale X      multiply the count by X (default: 1)"},
        {"-shout", 0, "-shout        use capitals"},
        {"-h", 0, "-h            show this help"},
    };

    void print_usage()
    {
        hsd::println("usage: greeter [options]"_fmt);
        for (const auto& option : options)
        {
            hsd::println("  {}"_fmt, option.help);
        }
    }

    // parse() stops safely on an unknown option or a missing value, but a clear
    // message is nicer than its usage dump, so the whole command line is checked first.
    bool command_line_is_valid(int argc, char** argv)
    {
        for (int index = 1; index < argc; )
        {
            const option_info* found = nullptr;

            for (const auto& option : options)
            {
                if (hsd::cstring::compare(argv[index], option.name) == 0)
                {
                    found = &option;
                }
            }

            if (found == nullptr)
            {
                hsd::println("unknown option: {}"_fmt, argv[index]);
                return false;
            }

            if (index + static_cast<int>(found->token_count) >= argc)
            {
                hsd::println("missing value for {}"_fmt, argv[index]);
                return false;
            }

            index += 1 + static_cast<int>(found->token_count);
        }

        return true;
    }

    char to_upper(char ch)
    {
        return (ch >= 'a' && ch <= 'z') ? static_cast<char>(ch - 'a' + 'A') : ch;
    }
}

int main(int argc, char** argv)
{
    // parse() must not read past the end of argv when a value is missing. This
    // command line is one value short for -pair, so its callback must not run.
    {
        hsd::argument_parser checker = "checker"_sv;
        hsd::i32 pair_calls = 0;
        checker.add("-pair", 2, [&pair_calls](hsd::parser_stream&) { ++pair_calls; }, "-pair a b");

        const char* short_argv[] = {"cli", "-pair", "1"};
        checker.parse(3, short_argv);
        hsd::println("short command line ran {} callbacks"_fmt, pair_calls);
    }

    if (command_line_is_valid(argc, argv) == false)
    {
        return 0;
    }

    // parse() prints its own help block with printf when the first argument is
    // -h, which would mix with our output, so answer that case here.
    if (argc > 1 && hsd::cstring::compare(argv[1], "-h") == 0)
    {
        print_usage();
        return 0;
    }

    hsd::string name = hsd::string{"world"};
    hsd::i32 times = 1;
    hsd::f32 scale = 1.0f;
    bool shout = false;
    bool bad_value = false;
    bool show_help = false;

    hsd::argument_parser parser = "Greets someone a few times"_sv;

    parser.add("-name", 1,
        [&](hsd::parser_stream& stream) { bad_value |= !stream.set_data(name).is_ok(); },
        options[0].help);
    parser.add("-times", 1,
        [&](hsd::parser_stream& stream) { bad_value |= !stream.set_data(times).is_ok(); },
        options[1].help);
    parser.add("-scale", 1,
        [&](hsd::parser_stream& stream) { bad_value |= !stream.set_data(scale).is_ok(); },
        options[2].help);
    parser.add("-shout", 0,
        [&](hsd::parser_stream&) { shout = true; },
        options[3].help);
    // -h is registered so that it is accepted later in the line too.
    parser.add("-h", 0,
        [&](hsd::parser_stream&) { show_help = true; },
        options[4].help);

    // main gives char**, while parse() takes const char**.
    parser.parse(argc, const_cast<const char**>(argv));

    if (show_help)
    {
        print_usage();
        return 0;
    }

    if (bad_value)
    {
        hsd::println("error: a value could not be read"_fmt);
        return 0;
    }

    hsd::i32 count = static_cast<hsd::i32>(static_cast<hsd::f32>(times) * scale);

    if (count < 1 || count > 20)
    {
        hsd::println("error: count {} is outside 1..20"_fmt, count);
        return 0;
    }

    hsd::string greeting = hsd::string{"hello, "};
    greeting += name;
    greeting += hsd::string{"!"};

    if (shout)
    {
        for (hsd::usize i = 0; i < greeting.length(); ++i)
        {
            greeting[i] = to_upper(greeting[i]);
        }
    }

    for (hsd::i32 line = 1; line <= count; ++line)
    {
        hsd::println("{}: {}"_fmt, line, greeting);
    }

    return 0;
}
