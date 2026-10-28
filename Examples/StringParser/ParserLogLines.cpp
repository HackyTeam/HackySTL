// Title: Parsing log lines
// Level: Intermediate
// Shows: parse_string over a list of lines, per-level tallies, and skipping malformed lines.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/StringParser.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

int main()
{
    const char* lines[] = {
        "[09:15:02] INFO net: link up",
        "[09:15:40] WARN disk: 91% full",
        "[09:16:11] ERROR net: timeout after 30s",
        "garbage without a timestamp",
        "[09:17:05] INFO db: connected",
        "[09:17:59] ERROR disk: write failed",
        "[10:02:00] WARN net: retrying",
    };

    hsd::usize info_count = 0, warn_count = 0, error_count = 0, bad_count = 0;
    hsd::u32 last_minutes = 0;

    // A placeholder ends at the character that follows it in the pattern, and
    // the last one takes the rest of the line.
    for (const char* line : lines)
    {
        auto parsed = hsd::parse_string<"[{}:{}:{}] {} {}: {}", 6>(line);

        if (parsed.is_ok() == false)
        {
            ++bad_count;
            hsd::println("skipped: {}"_fmt, line);
            continue;
        }

        auto fields = parsed.unwrap();
        hsd::string_view level = fields[3].to_string_view();

        if (level == "ERROR"_sv)
        {
            ++error_count;
            hsd::println("error from {} at {}:{}: {}"_fmt,
                fields[4].to_string(), fields[0].to_u32(),
                fields[1].to_u32(), fields[5].to_string());
        }
        else if (level == "WARN"_sv)
        {
            ++warn_count;
        }
        else
        {
            ++info_count;
        }

        // Fields convert on demand, so the clock can be rebuilt as minutes.
        last_minutes = fields[0].to_u32() * 60 + fields[1].to_u32();
    }

    hsd::println("counts: info {}, warn {}, error {}, skipped {}"_fmt,
        info_count, warn_count, error_count, bad_count);
    hsd::println("last entry at minute {} of the day"_fmt, last_minutes);

    return 0;
}
