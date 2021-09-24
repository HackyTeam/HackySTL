// This file is UTF-8

#include <cassert>
#include <Extra/Json.hpp>
#include <Io.hpp>

using namespace hsd::string_view_literals;

static constexpr auto s_test_string =
LR"json({
    "Cow": "milk",
    "gold": 5000,
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
        hsd_print(L"<root>");
    else
    {
        for (auto part : path)
            hsd_print(L"{}:", part);
    }
    hsd_print(L" ");
}

void print_value(hsd::vector<hsd::wstring_view>& path, hsd::JsonValue& v)
{
    print_prefix(path);
    hsd::JsonValueType type = v.type();

    switch (type)
    {
        case hsd::JsonValueType::Null:
            hsd_print(L"null");
            break;
        case hsd::JsonValueType::True:
            hsd_print(L"true");
            break;
        case hsd::JsonValueType::False:
            hsd_print(L"false");
            break;
        case hsd::JsonValueType::Number:
            hsd_print(L"{}", v.as_num<hsd::i32>().unwrap());
            break;
        case hsd::JsonValueType::String:
            // TODO: Escape?
            hsd_print(L"\"{}\"", v.as_str<hsd::wchar>().unwrap());
            break;
        case hsd::JsonValueType::Array:
            hsd_print(L"array");
            break;
        case hsd::JsonValueType::Object:
            hsd_print(L"object");
            break;
        }
    hsd_println(L"");

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
