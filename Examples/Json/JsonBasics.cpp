// Title: JSON basics
// Level: Basic
// Shows: lexing and parsing a JSON string, then reading typed fields with checked access.

#include <Extra/Json.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

int main()
{
    auto source = R"json({"name": "pumpkin", "count": 12, "price": 2.5, "fresh": true, "tags": ["orange", "round"]})json"_sv;

    // The lexer turns text into tokens. push_eot() tells it that the input is over.
    hsd::JsonStream lexer;
    lexer.lex(source).unwrap();
    lexer.push_eot().unwrap();

    // The parser consumes the tokens and builds an owning value tree.
    hsd::JsonParser parser = lexer;
    auto root = parser.parse_next().unwrap();

    // operator[] is fine here because every key is known to exist.
    hsd::println("name: {}"_fmt, (*root)["name"_sv].as_str().unwrap());
    hsd::println("count: {}"_fmt, (*root)["count"_sv].as_num<hsd::i32>().unwrap());
    hsd::println("price: {prec=2}"_fmt, (*root)["price"_sv].as_num<hsd::f32>().unwrap());
    hsd::println("fresh: {}"_fmt, (*root)["fresh"_sv].as_bool().unwrap() ? "true" : "false");

    auto& tags = (*root)["tags"_sv];
    hsd::println("tags: {}"_fmt, tags.as_array().size());
    hsd::println("first tag: {}"_fmt, tags[0].as_str().unwrap());

    // Typed access is checked: asking for the wrong type gives a failed result.
    auto wrong = (*root)["name"_sv].as_num<hsd::i32>();
    hsd::println("name as number ok: {}"_fmt, wrong.is_ok() ? "true" : "false");

    // access() is the safe lookup for keys that may be missing.
    auto missing = root->access("color"_sv);
    hsd::println("color found: {}"_fmt, missing.is_ok() ? "true" : "false");

    return 0;
}
