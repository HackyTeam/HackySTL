// This file is UTF-8

#include <Extra/Json.hpp>
#include <Io.hpp>
#include <cassert>

using namespace hsd::string_view_literals;

static constexpr auto s_test_string =
LR"json({
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

void print_prefix(hsd::vector<hsd::wstring_view>& path);
void print_value(hsd::vector<hsd::wstring_view>& path, hsd::JsonValue& v);

int main()
{
    hsd::JsonStream<hsd::wchar> lexer;
    // Lex the string
    lexer.lex(s_test_string).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser = lexer;
    // Parse the string
    auto value = parser.parse_next().unwrap();

    // Assert that the whole object is complete
    assert(value->is_complete());

    // Traverse the object
    hsd::vector<hsd::wstring_view> path;
    print_value(path, *value);
}

void print_prefix(hsd::vector<hsd::wstring_view>& path)
{
    if (path.size() == 0)
    {
        hsd_wprint(L"<root>");
    }
    else
    {
        for (auto part : path)
            hsd_wprint(L"{}:", part);
    }

    hsd_wprint(L" ");
}

void print_value(hsd::vector<hsd::wstring_view>& path, hsd::JsonValue& v)
{
    print_prefix(path);
    hsd::JsonValueType type = v.type();

    switch (type)
    {
        case hsd::JsonValueType::Null:
            hsd_wprint(L"null");
            break;
        case hsd::JsonValueType::True:
            hsd_wprint(L"true");
            break;
        case hsd::JsonValueType::False:
            hsd_wprint(L"false");
            break;
        case hsd::JsonValueType::Number:
        {
            auto res = v.as_num<hsd::i32>();

            if (res.is_ok())
            {
                hsd_wprint(L"{}", res.unwrap());
            }
            else
            {
                auto res = v.as_num<hsd::f32>();

                if (res.is_ok())
                {
                    hsd_wprint(L"{}", res.unwrap());
                }
                else
                {
                    hsd_wprint(L"<number>");
                }
            }
        }
            break;
        case hsd::JsonValueType::String:
            // TODO: Escape?
            hsd_wprint(L"\"{}\"", v.as_str<hsd::wchar>().unwrap());
            break;
        case hsd::JsonValueType::Array:
            hsd_wprint(L"array");
            break;
        case hsd::JsonValueType::Object:
            hsd_wprint(L"object");
            break;

        default:
            hsd_wprint(L"unknown");
            break;
    }

    hsd_wprintln(L"");

    if (type == hsd::JsonValueType::Array)
    {
        auto& vec = v.as<hsd::JsonArray>().values();

        for (hsd::usize idx = 0; auto& it : vec)
        {
            auto s = hsd::to_wstring(idx);
            path.push_back(static_cast<hsd::wstring_view>(s));
            print_value(path, *it);
            path.pop_back();
            idx++;
        }
    }
    else if (type == hsd::JsonValueType::Object)
    {
        auto& map = v.as<hsd::JsonObject<hsd::wchar>>().values();
        
        for (auto& kv : map)
        {
            path.push_back(static_cast<hsd::wstring_view>(kv.first));
            print_value(path, *kv.second);
            path.pop_back();
        }
    }
}
