// This file is UTF-8

#include <Extra/Json.hpp>
#include <Io.hpp>
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
    "primitives": [null, true, false]
})json"_sv;

void print_prefix(hsd::vector<hsd::string_view>& path);
void print_value(hsd::vector<hsd::string_view>& path, hsd::JsonValue& v);

int main()
{
    hsd::JsonStream<char> lexer;
    // Lex the string
    lexer.lex(s_test_string).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser = lexer;
    // Parse the string
    auto value = parser.parse_next().unwrap();

    // Assert that the whole object is complete
    assert(value->is_complete());

    // Traverse the object
    hsd::vector<hsd::string_view> path;
    print_value(path, *value);
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
            hsd::print("{}:"_fmt, part);
        }
    }

    hsd::print(" "_fmt);
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
                auto res = v.as_num<hsd::f32>();

                if (res.is_ok())
                {
                    hsd::print("{}"_fmt, res.unwrap());
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
            hsd::print("\"{}\""_fmt, v.as_str<char>().unwrap());
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
        auto& map = v.as<hsd::JsonObject<char>>().values();
        
        for (auto& kv : map)
        {
            path.push_back(static_cast<hsd::string_view>(kv.first));
            print_value(path, *kv.second);
            path.pop_back();
        }
    }
}
