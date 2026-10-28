// This file is UTF-8

#include <Extra/Json.hpp>
#include <Serialize/Io.hpp>
#include <cassert>

using namespace hsd::string_view_literals;
using namespace hsd::format_literals;

static constexpr auto s_test_string =
R"json({
    "Cow": "milk",
    "gold": 5000,
    "price": 121.5423,
    "primes": [2, 3, 5, 7, 11, 13, 23],
    "Unicode": "蛋",
    "MontyPython": "import pickle",
    "LoS": [
        {
            "name": "A",
            "age": 10
        },
        {
            "name": "B",
            "age": 20
        }
    ],
    "SoA": {
        "name": ["A", "B"],
        "age": [10, 20]
    },
    "primitives": [null, true, false],
    "negative": -42,
    "floatIntegerLike": 10.0,
    "limits": [2147483647, -2147483648, 2147483648, -2147483649],
    "emptyString": "",
    "emptyArray": [],
    "emptyObject": {},
    "matrix": [[1, 2], [3, 4]]
})json"_sv;

void print_prefix(hsd::vector<hsd::string_view>& path);
void print_value(hsd::vector<hsd::string_view>& path, hsd::JsonValue& v);

static hsd::unique_ptr<hsd::JsonValue> parse_json(hsd::string_view input)
{
    hsd::JsonStream lexer;
    lexer.lex(input).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser = lexer;
    auto result = parser.parse_next();
    assert(result.is_ok());
    return result.unwrap();
}

static void test_scalar_roots_and_multiple_values()
{
    auto null_value = parse_json("null"_sv);
    assert(null_value->is_complete());
    assert(null_value->type() == hsd::JsonValueType::Null);
    assert(!null_value->as_bool().is_ok());

    auto true_value = parse_json("true"_sv);
    assert(true_value->type() == hsd::JsonValueType::True);
    assert(true_value->as_bool().unwrap());

    auto false_value = parse_json("false"_sv);
    assert(false_value->type() == hsd::JsonValueType::False);
    assert(!false_value->as_bool().unwrap());

    auto number_value = parse_json("-17"_sv);
    assert(number_value->as_num<hsd::i32>().unwrap() == -17);

    auto string_value = parse_json("\"root string\""_sv);
    assert(string_value->as_str().unwrap() == "root string"_sv);

    auto empty_array = parse_json("[]"_sv);
    assert(empty_array->type() == hsd::JsonValueType::Array);
    assert(empty_array->as_array().size() == 0);

    auto empty_object = parse_json("{}"_sv);
    assert(empty_object->type() == hsd::JsonValueType::Object);
    assert(empty_object->as_object().size() == 0);

    hsd::JsonStream lexer;
    lexer.lex("true 12 \"tail\""_sv).unwrap();
    lexer.push_eot().unwrap();
    hsd::JsonParser parser = lexer;

    auto first = parser.parse_next();
    assert(first.is_ok());
    assert(first.unwrap()->as_bool().unwrap());

    auto second = parser.parse_next();
    assert(second.is_ok());
    assert(second.unwrap()->as_num<hsd::i32>().unwrap() == 12);

    auto third = parser.parse_next();
    assert(third.is_ok());
    assert(third.unwrap()->as_str().unwrap() == "tail"_sv);
    assert(!parser.parse_next().is_ok());
}

static void test_incremental_lexing_and_errors()
{
    hsd::JsonStream lexer;
    lexer.lex("{\"word\":\"seg"_sv).unwrap();
    lexer.lex("mented\",\"count\":-4"_sv).unwrap();
    lexer.lex("2,\"ratio\":1.25}"_sv).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser = lexer;
    auto value = parser.parse_next().unwrap();
    assert((*value)["word"_sv].as_str().unwrap() == "segmented"_sv);
    assert((*value)["count"_sv].as_num<hsd::i32>().unwrap() == -42);
    assert((*value)["ratio"_sv].as_num<hsd::f32>().unwrap() == 1.25f);

    hsd::JsonStream invalid_character;
    assert(!invalid_character.lex("@"_sv).is_ok());

    hsd::JsonStream unterminated_string;
    unterminated_string.lex("\"unfinished"_sv).unwrap();
    assert(!unterminated_string.push_eot().is_ok());

    hsd::JsonStream missing_colon;
    missing_colon.lex("{\"key\" 1}"_sv).unwrap();
    missing_colon.push_eot().unwrap();
    hsd::JsonParser malformed_parser = missing_colon;
    assert(!malformed_parser.parse_next().is_ok());

    hsd::JsonStream trailing_comma;
    trailing_comma.lex("[1,]"_sv).unwrap();
    trailing_comma.push_eot().unwrap();
    hsd::JsonParser trailing_comma_parser = trailing_comma;
    assert(!trailing_comma_parser.parse_next().is_ok());
}

int main()
{
    hsd::JsonStream lexer;
    // Lex the string
    lexer.lex(s_test_string).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser = lexer;
    // Parse the string
    auto value = parser.parse_next().unwrap();

    // Assert that the whole object is complete
    assert(value->is_complete());
    assert(value->type() == hsd::JsonValueType::Object);
    assert(value->as_object().size() == 16);

    assert((*value)["Cow"_sv].type() == hsd::JsonValueType::String);
    assert((*value)["Cow"_sv].as_str().unwrap() == "milk"_sv);
    assert((*value)["Unicode"_sv].as_str().unwrap() == "蛋"_sv);
    assert((*value)["MontyPython"_sv].as_str().unwrap() == "import pickle"_sv);
    assert((*value)["emptyString"_sv].as_str().unwrap().length() == 0);

    assert((*value)["gold"_sv].as_num<hsd::i32>().unwrap() == 5000);
    assert((*value)["negative"_sv].as_num<hsd::i32>().unwrap() == -42);
    assert((*value)["price"_sv].as_num<hsd::f32>().unwrap() > 121.5f);
    assert((*value)["floatIntegerLike"_sv].as_num<hsd::f32>().unwrap() == 10.0f);
    assert(!(*value)["floatIntegerLike"_sv].as_num<hsd::i32>().is_ok());
    assert(!(*value)["gold"_sv].as_num<hsd::f32>().is_ok());
    assert(!(*value)["price"_sv].as_num<hsd::i32>().is_ok());
    assert(!(*value)["gold"_sv].as_str().is_ok());
    assert(!(*value)["gold"_sv].as_bool().is_ok());

    assert((*value)["primes"_sv].type() == hsd::JsonValueType::Array);
    assert((*value)["primes"_sv].as_array().size() == 7);
    assert((*value)["primes"_sv][0].as_num<hsd::i32>().unwrap() == 2);
    assert((*value)["primes"_sv][1].as_num<hsd::i32>().unwrap() == 3);
    assert((*value)["primes"_sv][2].as_num<hsd::i32>().unwrap() == 5);
    assert((*value)["primes"_sv][3].as_num<hsd::i32>().unwrap() == 7);
    assert((*value)["primes"_sv][4].as_num<hsd::i32>().unwrap() == 11);
    assert((*value)["primes"_sv][5].as_num<hsd::i32>().unwrap() == 13);
    assert((*value)["primes"_sv][6].as_num<hsd::i32>().unwrap() == 23);

    assert((*value)["LoS"_sv][1]["age"_sv].as_num<hsd::i32>().unwrap() == 20);
    assert((*value)["LoS"_sv][0]["name"_sv].as_str().unwrap() == "A"_sv);
    assert((*value)["LoS"_sv][0]["age"_sv].as_num<hsd::i32>().unwrap() == 10);
    assert((*value)["LoS"_sv][1]["name"_sv].as_str().unwrap() == "B"_sv);
    assert((*value)["SoA"_sv]["name"_sv].as_array().size() == 2);
    assert((*value)["SoA"_sv]["name"_sv][0].as_str().unwrap() == "A"_sv);
    assert((*value)["SoA"_sv]["name"_sv][1].as_str().unwrap() == "B"_sv);
    assert((*value)["SoA"_sv]["age"_sv][0].as_num<hsd::i32>().unwrap() == 10);
    assert((*value)["SoA"_sv]["age"_sv][1].as_num<hsd::i32>().unwrap() == 20);

    assert((*value)["primitives"_sv][0].type() == hsd::JsonValueType::Null);
    assert(!(*value)["primitives"_sv][0].as_bool().is_ok());
    assert((*value)["primitives"_sv][1].type() == hsd::JsonValueType::True);
    assert((*value)["primitives"_sv][1].as_bool().unwrap());
    assert((*value)["primitives"_sv][2].type() == hsd::JsonValueType::False);
    assert(!(*value)["primitives"_sv][2].as_bool().unwrap());

    auto& limits = (*value)["limits"_sv];
    assert(limits[0].as_num<hsd::i32>().unwrap() == 2147483647);
    assert(limits[1].as_num<hsd::i32>().unwrap() == -2147483647 - 1);
    assert(limits[2].as_num<hsd::i64>().unwrap() == 2147483648LL);
    assert(limits[3].as_num<hsd::i64>().unwrap() == -2147483649LL);
    assert(!limits[2].as_num<hsd::i32>().is_ok());
    assert(!limits[3].as_num<hsd::i32>().is_ok());

    assert((*value)["emptyArray"_sv].as_array().size() == 0);
    assert((*value)["emptyObject"_sv].as_object().size() == 0);
    assert((*value)["matrix"_sv][0].as_array().size() == 2);
    assert((*value)["matrix"_sv][0][0].as_num<hsd::i32>().unwrap() == 1);
    assert((*value)["matrix"_sv][0][1].as_num<hsd::i32>().unwrap() == 2);
    assert((*value)["matrix"_sv][1][0].as_num<hsd::i32>().unwrap() == 3);
    assert((*value)["matrix"_sv][1][1].as_num<hsd::i32>().unwrap() == 4);

    auto cow_access = value->access("Cow"_sv);
    assert(cow_access.is_ok());
    assert(cow_access.unwrap().get().as_str().unwrap() == "milk"_sv);
    assert(!value->access("missing"_sv).is_ok());
    assert(!value->access(static_cast<hsd::usize>(0)).is_ok());
    assert(!(*value)["primes"_sv].access("first"_sv).is_ok());
    assert(!(*value)["primes"_sv].access(static_cast<hsd::usize>(7)).is_ok());

    // Traverse the object
    hsd::vector<hsd::string_view> path;
    print_value(path, *value);

    test_scalar_roots_and_multiple_values();
    test_incremental_lexing_and_errors();
}

void print_prefix(hsd::vector<hsd::string_view>& path)
{
    if (path.size() == 0)
    {
        hsd::print("<root>"_fmt);
    }
    else
    {
        for (auto part : path)
        {
            hsd::print("{}: "_fmt, part);
        }
    }
}

void print_value(hsd::vector<hsd::string_view>& path, hsd::JsonValue& v)
{
    print_prefix(path);
    hsd::JsonValueType type = v.type();

    switch (type)
    {
        case hsd::JsonValueType::Null:
        {
            hsd::print("null"_fmt);
            break;
        }
        case hsd::JsonValueType::True:
        {
            hsd::print("true"_fmt);
            break;
        }
        case hsd::JsonValueType::False:
        {
            hsd::print("false"_fmt);
            break;
        }
        case hsd::JsonValueType::Number:
        {
            auto res = v.as_num<hsd::i32>();

            if (res.is_ok())
            {
                hsd::print("{}"_fmt, res.unwrap());
            }
            else
            {
                auto float_res = v.as_num<hsd::f32>();

                if (float_res.is_ok())
                {
                    hsd::print("{}"_fmt, float_res.unwrap());
                }
                else
                {
                    hsd::print("<number>"_fmt);
                }
            }

            break;
        }
        case hsd::JsonValueType::String:
        {
            // TODO: Escape?
            auto text = v.as_str().unwrap();
            if (text.length() == 0)
                hsd::print("\"\""_fmt);
            else
                hsd::print("\"{}\""_fmt, text);
            break;
        }
        case hsd::JsonValueType::Array:
        {
            hsd::print("array"_fmt);
            break;
        }
        case hsd::JsonValueType::Object:
        {
            hsd::print("object"_fmt);
            break;
        }
        default:
        {
            hsd::print("unknown"_fmt);
            break;
        }
    }

    hsd::print("\n"_fmt);

    if (type == hsd::JsonValueType::Array)
    {
        auto& vec = v.as<hsd::JsonArray>().values();

        for (hsd::usize idx = 0; auto& it : vec)
        {
            auto s = hsd::to_string(idx);
            path.push_back(static_cast<hsd::string_view>(s));
            print_value(path, *it);
            path.pop_back();
            idx++;
        }
    }
    else if (type == hsd::JsonValueType::Object)
    {
        auto& map = v.as<hsd::JsonObject>().values();
        
        for (auto& kv : map)
        {
            path.push_back(static_cast<hsd::string_view>(kv.first));
            print_value(path, *kv.second);
            path.pop_back();
        }
    }
}
