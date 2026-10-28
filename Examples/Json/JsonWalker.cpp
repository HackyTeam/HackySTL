// Title: JSON tree walker
// Level: Intermediate
// Shows: lexing input in fragments, walking a nested tree recursively, and reporting syntax errors.

#include <Extra/Json.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

static void print_indent(hsd::usize depth)
{
    for (hsd::usize i = 0; i < depth; i++)
    {
        hsd::print("  "_fmt);
    }
}

static void walk(hsd::JsonValue& value, hsd::usize depth)
{
    switch (value.type())
    {
        case hsd::JsonValueType::Null:
            hsd::println("null"_fmt);
            break;
        case hsd::JsonValueType::True:
            hsd::println("true"_fmt);
            break;
        case hsd::JsonValueType::False:
            hsd::println("false"_fmt);
            break;
        case hsd::JsonValueType::Number:
        {
            // Integers convert to i32 directly; anything else is read as a float.
            auto integer = value.as_num<hsd::i32>();

            if (integer.is_ok())
                hsd::println("{}"_fmt, integer.unwrap());
            else
                hsd::println("{prec=3}"_fmt, value.as_num<hsd::f32>().unwrap_or(0.f));
            break;
        }
        case hsd::JsonValueType::String:
            hsd::println("\"{}\""_fmt, value.as_str().unwrap());
            break;
        case hsd::JsonValueType::Array:
        {
            hsd::println("array({})"_fmt, value.as_array().size());

            for (hsd::usize i = 0; auto& item : value.as_array())
            {
                print_indent(depth + 1);
                hsd::print("[{}] "_fmt, i++);
                walk(*item, depth + 1);
            }
            break;
        }
        case hsd::JsonValueType::Object:
        {
            hsd::println("object({})"_fmt, value.as_object().size());

            // Members come back in the order they were written.
            for (auto& member : value.as_object())
            {
                print_indent(depth + 1);
                hsd::print("{}: "_fmt, member.first);
                walk(*member.second, depth + 1);
            }
            break;
        }
        default:
            hsd::println("unknown"_fmt);
            break;
    }
}

static bool parses(hsd::string_view text)
{
    hsd::JsonStream lexer;

    if (!lexer.lex(text).is_ok() || !lexer.push_eot().is_ok())
        return false;

    hsd::JsonParser parser = lexer;
    return parser.parse_next().is_ok();
}

int main()
{
    // The lexer keeps its state between calls, so a document may arrive in pieces,
    // even in the middle of a string or a number.
    hsd::JsonStream lexer;
    lexer.lex(R"json({"city": "Ly)json"_sv).unwrap();
    lexer.lex(R"json(on", "pop": 5)json"_sv).unwrap();
    lexer.lex(R"json(16, "ratio": 0.5, "tags": ["a", null, true],)json"_sv).unwrap();
    lexer.lex(R"json( "geo": {"lat": 45.75, "lon": 4.85}})json"_sv).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser = lexer;
    auto root = parser.parse_next().unwrap();
    hsd::print("root: "_fmt);
    walk(*root, 0);

    // Malformed input produces a failed result instead of a crash.
    hsd::println("valid array: {}"_fmt, parses("[1, 2, 3]"_sv) ? "true" : "false");
    hsd::println("trailing comma: {}"_fmt, parses("[1,]"_sv) ? "true" : "false");
    hsd::println("missing colon: {}"_fmt, parses("{\"key\" 1}"_sv) ? "true" : "false");
    hsd::println("bad character: {}"_fmt, parses("@"_sv) ? "true" : "false");

    return 0;
}
