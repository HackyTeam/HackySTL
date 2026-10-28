// Title: Log analyzer
// Level: Intermediate
// Shows: parse_string on log lines, per-service statistics and a filtered range of slow requests.

#include <Base/Algorithm.hpp>
#include <Containers/Range.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/StringParser.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

struct log_event
{
    hsd::string severity;
    hsd::string service;
    hsd::i32 latency_ms;
    hsd::i32 request_id;
};

struct service_stats
{
    hsd::string name;
    hsd::i32 requests = 0;
    hsd::i32 total_ms = 0;
    hsd::i32 errors = 0;

    hsd::f64 average() const
    {
        return static_cast<hsd::f64>(total_ms) / requests;
    }

    // Required by quick_sort's element constraint; the predicate sets the order.
    bool operator>(const service_stats& other) const
    {
        return average() > other.average();
    }
};

int main()
{
    constexpr const char* lines[] = {
        "[INFO] api 12 101",
        "[WARN] api 110 102",
        "[ERROR] db 45 103",
        "[INFO] auth 9 104",
        "this line is not a log entry",
        "[ERROR] api 240 105",
        "[WARN] db 75 106",
        "[INFO] auth 14 107",
        "[INFO] db 30",
        "[INFO] api 38 108"
    };

    hsd::vector<log_event> events;
    hsd::usize malformed = 0;

    // parse_string fails (not panics) when a line does not match the pattern.
    for (const char* line : lines)
    {
        auto fields = hsd::parse_string<"[{}] {} {} {}", 4>(line);
        if (!fields.is_ok())
        {
            ++malformed;
            continue;
        }

        auto parsed = fields.unwrap();
        events.push_back(log_event{
            parsed[0].to_string(),
            parsed[1].to_string(),
            parsed[2].to_i32(),
            parsed[3].to_i32()
        });
    }
    hsd::println("parsed {} lines, skipped {}"_fmt, events.size(), malformed);

    hsd::unordered_map<hsd::string, hsd::i32> severity_counts;
    hsd::unordered_map<hsd::string, service_stats> by_service;
    for (const auto& event : events)
    {
        ++severity_counts[event.severity];

        auto& stats = by_service[event.service];
        stats.name = event.service;
        ++stats.requests;
        stats.total_ms += event.latency_ms;
        if (event.severity == "ERROR"_sv)
        {
            ++stats.errors;
        }
    }

    hsd::println("severities: INFO={} WARN={} ERROR={}"_fmt,
        severity_counts["INFO"], severity_counts["WARN"], severity_counts["ERROR"]);

    // Slowest average first: `left` goes after `right` when it is faster.
    hsd::vector<service_stats> ranking;
    for (const auto& entry : by_service)
    {
        ranking.push_back(entry.second);
    }
    hsd::quick_sort(ranking, [](const service_stats& left, const service_stats& right)
    {
        return left.average() < right.average();
    });

    hsd::println("services by average latency:"_fmt);
    for (const auto& stats : ranking)
    {
        hsd::println("  {}: avg={prec=1}ms requests={} errors={}"_fmt,
            stats.name, stats.average(), stats.requests, stats.errors);
    }

    hsd::print("slow requests (>=100ms):"_fmt);
    for (const auto& event : events | hsd::ranges::filter(
        [](const log_event& item) { return item.latency_ms >= 100; }))
    {
        hsd::print(" #{}"_fmt, event.request_id);
    }
    hsd::println(""_fmt);
}
