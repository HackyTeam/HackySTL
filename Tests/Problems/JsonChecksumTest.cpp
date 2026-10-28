#include <cassert>

#include <Extra/Json.hpp>

using namespace hsd::string_view_literals;

struct json_metrics
{
    int sum = 0;
    int strings = 0;
    int max_depth = 0;
};

static void collect_metrics(
    hsd::JsonValue& value, int depth, json_metrics& metrics)
{
    metrics.max_depth = std::max(metrics.max_depth, depth);
    switch (value.type())
    {
        case hsd::JsonValueType::Number:
            metrics.sum += value.as_num<int>().unwrap();
            break;
        case hsd::JsonValueType::String:
            ++metrics.strings;
            break;
        case hsd::JsonValueType::Array:
            for (auto& item : value.as_array())
                collect_metrics(*item, depth + 1, metrics);
            break;
        case hsd::JsonValueType::Object:
            for (auto& [key, item] : value.as_object())
            {
                (void)key;
                collect_metrics(*item, depth + 1, metrics);
            }
            break;
        default:
            break;
    }
}

int main()
{
    constexpr auto input = R"json({
        "name": "example",
        "values": [1, 2, {"nested": [3, 4]}],
        "flags": [true, false, null],
        "meta": {"label": "deep", "score": 5}
    })json"_sv;

    hsd::JsonStream stream;
    stream.lex(input).unwrap();
    stream.push_eot().unwrap();
    hsd::JsonParser parser = stream;
    auto root = parser.parse_next().unwrap();

    json_metrics metrics;
    collect_metrics(*root, 0, metrics);
    assert(metrics.sum == 15);
    assert(metrics.strings == 2);
    assert(metrics.max_depth == 4);
    assert((*root)["values"_sv][2]["nested"_sv][1].as_num<int>().unwrap() == 4);
    assert(!(*root)["values"_sv].as_str().is_ok());
}