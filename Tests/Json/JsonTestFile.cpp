// This file is UTF-8

#include <Json.hpp>
#include <Io.hpp>

static const char* test_filename = "Tests/Json/TestFile.json";

int main() 
{
    setlocale(LC_ALL, "en_US.UTF-8");
    hsd::JsonStream<hsd::wchar> lexer;
    
    // Lex the string
    lexer.lex_file(test_filename).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser(lexer);
    // Parse the string
    auto value = parser.parse_next().unwrap();

    // Assert that the whole object is complete
    assert(value->is_complete());
    
    auto res = value->access(L"LoS").access(0).access(L"age").as_num();
}