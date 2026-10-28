// Title: String streams
// Level: Basic
// Shows: sstream write_data/append_data/set_data, a stream that grows from no reserve(), and a compile-time static_sstream.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/SStream.hpp>

using namespace hsd::format_literals;

// A static_sstream has a fixed capacity and can be filled during compilation.
static constexpr auto make_banner()
{
    hsd::static_sstream<64> stream;
    stream.write_data("{} + {} = {}"_fmt, 2, 3, 5);
    return stream;
}

static constexpr auto banner = make_banner();
static_assert(banner.size() == 9);

int main()
{
    hsd::sstream stream;
    stream.reserve(128);

    // write_data replaces the contents, append_data adds to them.
    stream.write_data("id={} "_fmt, 7);
    stream.append_data("ratio={prec=2} "_fmt, 0.5);
    stream.append_data("name={}"_fmt, "kiwi");

    // The stream converts to a string_view over what was written.
    hsd::string_view written = stream;
    hsd::println("written: {}"_fmt, written);
    hsd::println("size: {}"_fmt, stream.size());

    // set_data splits the contents on whitespace and converts each field. The
    // last field needs a separator after it, so the text ends with a space.
    hsd::sstream input;
    input.reserve(64);
    input.write_data("12 3.5 apple "_fmt);

    hsd::i32 count = 0;
    hsd::f32 weight = 0;
    hsd::string fruit;
    auto parsed = input.set_data(count, weight, fruit);

    if (parsed.is_ok())
    {
        hsd::println("parsed: {} {prec=1} {}"_fmt, count, weight, fruit);
    }
    else
    {
        hsd::println("could not parse the stream"_fmt);
    }

    // Asking for more fields than the text holds is an error, not a crash.
    hsd::sstream short_input;
    short_input.reserve(32);
    short_input.write_data("5 "_fmt);
    hsd::i32 first = 0, second = 0;
    hsd::println("too few fields: {}"_fmt, short_input.set_data(first, second).is_ok() ? "ok" : "error");

    // The compile-time stream was built before main started.
    hsd::string_view banner_view = banner;
    hsd::println("compile time: {}"_fmt, banner_view);

    // A stream that never called reserve() has no storage yet. A value longer than
    // the buffer grows the storage, so every character is kept.
    hsd::string filler;
    for (hsd::usize index = 0; index < 1500; ++index)
    {
        filler.push_back('x');
    }

    hsd::sstream grown;
    grown.write_data("[{}]"_fmt, filler);
    hsd::println("grown: {} characters"_fmt, grown.size());

    return 0;
}
