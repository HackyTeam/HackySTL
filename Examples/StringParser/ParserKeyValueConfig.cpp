// Title: Key-value config reader
// Level: Advanced
// Shows: reading config lines from stdin and combining parse_string patterns for sections and keys.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/StringParser.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

namespace
{
    struct settings
    {
        hsd::string name = hsd::string{"unnamed"};
        hsd::u32 port = 0;
        hsd::f32 ratio = 1.0f;
        bool verbose = false;
        hsd::u32 unknown_keys = 0;
        hsd::u32 bad_lines = 0;
    };

    // Applies one "key: value" pair. Returns false if the value is not valid
    // for that key, so the caller can count the line as bad.
    bool apply(settings& config, const hsd::string_view& key, const hsd::string_view& value)
    {
        if (key == "name"_sv)
        {
            config.name = hsd::string{value};
        }
        else if (key == "port"_sv)
        {
            hsd::u32 port = hsd::cstring::parse<hsd::u32>(value.begin(), value.end());
            if (port == 0 || port > 65535)
            {
                return false;
            }

            config.port = port;
        }
        else if (key == "ratio"_sv)
        {
            config.ratio = hsd::cstring::parse<hsd::f32>(value.begin(), value.end());
        }
        else if (key == "verbose"_sv)
        {
            if (value != "yes"_sv && value != "no"_sv)
            {
                return false;
            }

            config.verbose = value == "yes"_sv;
        }
        else
        {
            ++config.unknown_keys;
        }

        return true;
    }
}

// Reads all of stdin one line at a time with read_line(). Every line, including
// blank lines and a final line without a newline, is kept, and each one ends with
// '\n'. A single read can return only part of a pipe, so the loop is needed.
hsd::string read_script()
{
    hsd::string text;
    auto line = hsd::io::cin().read_line();

    while (line.is_ok())
    {
        text += line.unwrap();
        text += "\n";
        line = hsd::io::cin().read_line();
    }

    return text;
}

int main()
{
    settings config;
    hsd::string section = hsd::string{"global"};
    hsd::usize line_number = 0;

    // The whole input is read in one chunk and split on newlines by hand, so
    // empty lines and a missing final newline need no special handling.
    const hsd::string script = read_script();
    hsd::string_view text{script.c_str(), script.length()};

    for (hsd::usize start = 0; start < text.length(); )
    {
        hsd::usize end = text.find('\n', start);
        if (end == hsd::string_view::npos)
        {
            end = text.length();
        }

        hsd::string line{text.sub_string(start, end - start).unwrap()};
        start = end + 1;
        ++line_number;

        if (line.length() == 0 || line.c_str()[0] == '#')
        {
            continue;
        }

        // Two patterns cover the two kinds of line. Try the section header
        // first, then the key and value form.
        auto header = hsd::parse_string<"[{}]", 1>(line.c_str());
        if (header.is_ok())
        {
            section = header.unwrap()[0].to_string();
            hsd::println("section {}"_fmt, section);
            continue;
        }

        auto pair = hsd::parse_string<"{}: {}", 2>(line.c_str());
        if (pair.is_ok() == false)
        {
            ++config.bad_lines;
            hsd::println("line {}: cannot read '{}'"_fmt, line_number, line);
            continue;
        }

        auto fields = pair.unwrap();
        if (apply(config, fields[0].to_string_view(), fields[1].to_string_view()) == false)
        {
            ++config.bad_lines;
            hsd::println("line {}: bad value for {}"_fmt, line_number, fields[0].to_string());
        }
    }

    hsd::println("last section: {}"_fmt, section);
    hsd::println("name: {}"_fmt, config.name);
    hsd::println("port: {}"_fmt, config.port);
    hsd::println("ratio: {prec=2}"_fmt, config.ratio);
    hsd::println("verbose: {}"_fmt, config.verbose ? "yes" : "no");
    hsd::println("unknown keys: {}, bad lines: {}"_fmt, config.unknown_keys, config.bad_lines);

    return 0;
}
