#include <cassert>
#include <Extra/Json.hpp>
#include <Io.hpp>

static const char* test_filename = "Tests/Json/TestFile.json";

int main() 
{
    using namespace hsd::string_view_literals;
    setlocale(LC_ALL, "en_US.UTF-8");
    hsd::JsonStream<char> lexer;
    
    // Lex the string
    lexer.lex_file(test_filename).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser(lexer);
    // Parse the string
    auto value = parser.parse_all().unwrap();

    // Assert that the whole object is complete
    assert(value->is_complete());
    
    hsd_println("Price {}", (*value)["price"_sv].as_num<hsd::f32>().unwrap());
}
