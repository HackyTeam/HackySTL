// Title: JSON inventory
// Level: Intermediate
// Shows: parsing JSON with JsonStream and JsonParser, then reading it with checked access.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Extra/Json.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

// Lexes the text and parses its first value. A broken document gives an error.
static hsd::result<hsd::unique_ptr<hsd::JsonValue>, hsd::JsonError>
parse_json(hsd::string_view text)
{
    hsd::JsonStream stream;
    auto lexed = stream.lex(text);
    if (!lexed.is_ok())
    {
        return lexed.unwrap_err();
    }
    stream.push_eot().unwrap();

    hsd::JsonParser parser = stream;
    return parser.parse_next();
}

// `access` returns a result, so a missing key is a value, not an abort.
static hsd::i32 number_or(hsd::JsonValue& object, hsd::string_view key, hsd::i32 fallback)
{
    auto field = object.access(key);
    if (!field.is_ok())
    {
        return fallback;
    }
    return field.unwrap().get().as_num<hsd::i32>().unwrap_or(fallback);
}

static void print_money(hsd::i32 cents)
{
    const hsd::i32 rest = cents % 100;
    hsd::print("{}.{}{}"_fmt, cents / 100, rest < 10 ? "0" : "", rest);
}

int main()
{
    constexpr auto document = R"json({
        "warehouse": "north",
        "reorder_below": 5,
        "items": [
            {"sku": "bolt",   "qty": 120, "price_cents": 15, "tags": ["metal", "small"]},
            {"sku": "gear",   "qty": 4,   "price_cents": 899, "tags": ["metal"]},
            {"sku": "belt",   "price_cents": 1250, "tags": []},
            {"sku": "washer", "qty": 5,   "price_cents": 8, "tags": ["metal", "small", "flat"]}
        ]
    })json"_sv;

    auto parsed = parse_json(document);
    if (!parsed.is_ok())
    {
        hsd::println("bad document: {}"_fmt, parsed.unwrap_err().pretty_error());
        return 0;
    }
    auto root = parsed.unwrap();

    const hsd::i32 threshold = number_or(*root, "reorder_below"_sv, 0);
    hsd::println("warehouse: {}"_fmt, (*root)["warehouse"_sv].as_str().unwrap());
    hsd::println("reorder below: {}"_fmt, threshold);

    hsd::i32 total_value = 0;
    hsd::i32 tag_count = 0;
    for (auto& item : (*root)["items"_sv].as_array())
    {
        // "belt" has no qty field, so it falls back to zero.
        const hsd::i32 quantity = number_or(*item, "qty"_sv, 0);
        const hsd::i32 price = number_or(*item, "price_cents"_sv, 0);
        const hsd::usize tags = (*item)["tags"_sv].as_array().size();

        hsd::print("{}: qty={} value="_fmt,
            (*item)["sku"_sv].as_str().unwrap(), quantity);
        print_money(quantity * price);
        hsd::println("{} tags={}"_fmt, quantity < threshold ? " REORDER" : "", tags);

        total_value += quantity * price;
        tag_count += static_cast<hsd::i32>(tags);
    }

    hsd::print("total value: "_fmt);
    print_money(total_value);
    hsd::println(""_fmt);
    hsd::println("total tags: {}"_fmt, tag_count);

    // A truncated document must be reported, not crash the program.
    auto broken = parse_json(R"json({"items": [1, 2)json"_sv);
    hsd::println("truncated document parsed: {}"_fmt,
        broken.is_ok() ? "true" : "false");
}
