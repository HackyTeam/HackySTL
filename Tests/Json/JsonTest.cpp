// This file is UTF-8

#include <Json.hpp>
#include <iostream>

static const hsd::wchar* s_test_string =
LR"json({
    "Cow": "milk",
    "gold": 5000,
    "primes": [2, 3, 5, 7, 11, 13],
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
})json";

void print_prefix(hsd::vector<hsd::wstring_view>& path);
void print_value(hsd::vector<hsd::wstring_view>& path, hsd::JsonValue& v);

int main() {
    hsd::JsonStream<hsd::wchar> lexer;
    // Lex the string
    lexer.lex(s_test_string).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser(lexer);
    // Parse the string
    auto value = parser.parse_next().unwrap();

    // Assert that the whole object is complete
    assert(value->is_complete());

    // Traverse the object
    hsd::vector<hsd::wstring_view> path;
    print_value(path, *value);
}

void print_prefix(hsd::vector<hsd::wstring_view>& path) {
    if (path.size() == 0)
        std::wcout << L"<root>";
    else {
        for (auto part : path)
            std::wcout << part << L':';
    }
    std::wcout << L' ';
}

void print_value(hsd::vector<hsd::wstring_view>& path, hsd::JsonValue& v) {
    print_prefix(path);
    hsd::JsonValueType type = v.type();
    switch (type) {
        case hsd::JsonValueType::Null:
            std::wcout << L"null";
            break;
        case hsd::JsonValueType::True:
            std::wcout << L"true";
            break;
        case hsd::JsonValueType::False:
            std::wcout << L"false";
            break;
        case hsd::JsonValueType::Number:
            std::wcout << v.as<hsd::JsonNumber>().value();
            break;
        case hsd::JsonValueType::String:
            // TODO: Escape?
            std::wcout << L'"' << v.as_str<hsd::wchar>().value() << L'"';
            break;
        case hsd::JsonValueType::Array:
            std::wcout << L"array";
            break;
        case hsd::JsonValueType::Object:
            std::wcout << L"object";
            break;
    }
    std::wcout << L'\n';

    if (type == hsd::JsonValueType::Array) {
        auto& vec = v.as<hsd::JsonArray>().values();
        hsd::usize idx = 0;
        for (auto it = vec.begin(), end = vec.end(); it != end; ++it, ++idx) {
            auto s = hsd::wstring::to_string(idx);
            path.push_back(static_cast<hsd::wstring_view>(s));
            print_value(path, *(*it));
            path.pop_back();
        }
    } else if (type == hsd::JsonValueType::Object) {
        auto& map = v.as<hsd::JsonObject<hsd::wchar>>().values();
        for (auto& kv : map) {
            path.push_back(static_cast<hsd::wstring_view>(kv.first));
            print_value(path, *kv.second);
            path.pop_back();
        }
    }
}