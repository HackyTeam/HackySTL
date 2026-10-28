#include <cassert>
#include <Extra/Json.hpp>
#include <Serialize/Io.hpp>

static const char* test_filename = "TestFile.json";

int main() 
{
    using namespace hsd::format_literals;
    using namespace hsd::string_view_literals;
    
    hsd::JsonStream lexer;
    
    // Lex the string
    lexer.lex_file(test_filename).unwrap();
    lexer.push_eot().unwrap();

    hsd::JsonParser parser = lexer;
    // Parse the string
    auto value = parser.parse_next().unwrap();

    // Assert that the whole object is complete
    assert(value->is_complete());
    assert(value->type() == hsd::JsonValueType::Object);
    assert(value->as_object().size() == 9);

    assert((*value)["price"_sv].as_num<hsd::f32>().unwrap() > 12.0f);
    assert((*value)["Cow"_sv].as_str().unwrap() == "milk"_sv);
    assert((*value)["gold"_sv].as_num<hsd::i32>().unwrap() == 5000);
    assert((*value)["Unicode"_sv].as_str().unwrap() == "蛋"_sv);
    assert((*value)["MontyPython"_sv].as_str().unwrap() == "import pickle"_sv);

    auto& primes = (*value)["primes"_sv];
    assert(primes.as_array().size() == 7);
    assert(primes[0].as_num<hsd::i32>().unwrap() == 2);
    assert(primes[6].as_num<hsd::i32>().unwrap() == 23);

    assert((*value)["LoS"_sv][0]["name"_sv].as_str().unwrap() == "A"_sv);
    assert((*value)["LoS"_sv][0]["age"_sv].as_num<hsd::i32>().unwrap() == 10);
    assert((*value)["LoS"_sv][1]["name"_sv].as_str().unwrap() == "B"_sv);
    assert((*value)["LoS"_sv][1]["age"_sv].as_num<hsd::i32>().unwrap() == 20);

    assert((*value)["SoA"_sv]["name"_sv][0].as_str().unwrap() == "A"_sv);
    assert((*value)["SoA"_sv]["name"_sv][1].as_str().unwrap() == "B"_sv);
    assert((*value)["SoA"_sv]["age"_sv][0].as_num<hsd::i32>().unwrap() == 10);
    assert((*value)["SoA"_sv]["age"_sv][1].as_num<hsd::i32>().unwrap() == 20);

    assert((*value)["primitives"_sv][0].type() == hsd::JsonValueType::Null);
    assert((*value)["primitives"_sv][1].as_bool().unwrap());
    assert(!(*value)["primitives"_sv][2].as_bool().unwrap());
}