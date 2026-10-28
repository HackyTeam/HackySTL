// Title: Printing custom types
// Level: Advanced
// Shows: the {ovr} tag with pretty_fmt and pretty_args, nested user types, and sstream output.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/SStream.hpp>

using namespace hsd::format_literals;

// A type prints with {ovr} when it provides two members:
//  - pretty_fmt<CharT>(): the format to use for the object, built at compile time.
//  - pretty_args(): a tuple with one value for each {} in that format.
struct point
{
    hsd::i32 x;
    hsd::i32 y;

    template <typename CharT>
    static consteval auto pretty_fmt()
    {
        static_assert(hsd::is_same<CharT, char>::value, "only narrow characters are supported here");

        constexpr auto fmt = "({}, {})"_fmt;
        return fmt();
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<hsd::i32, hsd::i32>
    {
        return {x, y};
    }
};

enum class color
{
    red,
    green,
    blue
};

static constexpr const char* color_name(color value)
{
    switch (value)
    {
        case color::red: return "red";
        case color::green: return "green";
        default: return "blue";
    }
}

// A type can contain other printable types: the {ovr} entries below are
// expanded with the nested type's own pretty_fmt.
struct segment
{
    point from;
    point to;
    color paint;

    template <typename CharT>
    static consteval auto pretty_fmt()
    {
        static_assert(hsd::is_same<CharT, char>::value, "only narrow characters are supported here");

        constexpr auto fmt = "segment {ovr} -> {ovr} in {}"_fmt;
        return fmt();
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<point, point, const char*>
    {
        return {from, to, color_name(paint)};
    }
};

struct measurement
{
    const char* label;
    hsd::f32 value;

    template <typename CharT>
    static consteval auto pretty_fmt()
    {
        static_assert(hsd::is_same<CharT, char>::value, "only narrow characters are supported here");

        // Tags such as {prec=N} work inside a custom format too.
        constexpr auto fmt = "{}: {prec=3}"_fmt;
        return fmt();
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<const char*, hsd::f32>
    {
        return {label, value};
    }
};

int main()
{
    point origin{0, 0};
    point corner{-3, 7};
    hsd::println("point: {ovr}"_fmt, corner);

    segment line{origin, corner, color::green};
    hsd::println("{ovr}"_fmt, line);

    // Custom values mix with ordinary ones in a single call.
    measurement reading{"voltage", 3.30004f};
    hsd::println("reading #{} is {ovr}, flag {}"_fmt, 4, reading, 'Y');

    // The same types print into an sstream, which is how a report line could
    // be built before writing it out.
    hsd::sstream report;
    report.reserve(256);
    report.write_data("from {ovr} to {ovr}"_fmt, origin, corner);
    hsd::string_view written = report;
    hsd::println("{} (size {})"_fmt, written, report.size());

    // A list of values is printed with a loop, one custom value per line.
    const segment path[] = {
        {{0, 0}, {1, 0}, color::red},
        {{1, 0}, {1, 1}, color::blue},
        {{1, 1}, {0, 0}, color::green},
    };

    for (const auto& piece : path)
    {
        hsd::println("  {ovr}"_fmt, piece);
    }

    return 0;
}
