#include <cassert>

#include <Containers/Range.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/StringParser.hpp>

using namespace hsd::string_view_literals;

struct log_event
{
    hsd::string severity;
    hsd::string service;
    hsd::i32 latency_ms;
    hsd::i32 request_id;
};

int main()
{
    constexpr const char* lines[] = {
        "[INFO] api 12 101",
        "[WARN] api 110 102",
        "[ERROR] db 45 103",
        "[INFO] auth 9 104",
        "[ERROR] api 240 105",
        "[WARN] db 75 106"
    };

    hsd::vector<log_event> events;
    hsd::unordered_map<hsd::string, hsd::i32> severity_counts;
    hsd::unordered_map<hsd::string, hsd::i32> service_latency;

    for (const char* line : lines)
    {
        auto fields = hsd::parse_string<"[{}] {} {} {}", 4>(line).unwrap();
        hsd::string severity = fields[0].to_string();
        hsd::string service = fields[1].to_string();
        const hsd::i32 latency = fields[2].to_i32();
        const hsd::i32 request_id = fields[3].to_i32();

        assert(latency >= 0);
        assert(request_id >= 100);
        assert(severity == hsd::string{"INFO"} ||
            severity == hsd::string{"WARN"} ||
            severity == hsd::string{"ERROR"});

        ++severity_counts[severity];
        service_latency[service] += latency;
        events.push_back({hsd::move(severity), hsd::move(service), latency, request_id});
    }

    assert(severity_counts.at("INFO").unwrap().get() == 2);
    assert(severity_counts.at("WARN").unwrap().get() == 2);
    assert(severity_counts.at("ERROR").unwrap().get() == 2);
    assert(service_latency.at("api").unwrap().get() == 362);
    assert(service_latency.at("db").unwrap().get() == 120);
    assert(service_latency.at("auth").unwrap().get() == 9);

    hsd::i32 slow_count = 0;
    hsd::i32 slow_api_count = 0;
    for (const auto& event : events | hsd::ranges::filter(
        [](const log_event& item) { return item.latency_ms >= 100; }
    ))
    {
        ++slow_count;
        if (event.service == "api"_sv)
        {
            ++slow_api_count;
        }
    }
    assert(slow_count == 2);
    assert(slow_api_count == 2);

    auto malformed = hsd::parse_string<"[{}] {} {} {}", 4>("[ERROR] db 103");
    assert(!malformed.is_ok());
}