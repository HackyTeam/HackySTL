#include <Serialize/StringParser.hpp>
#include <cassert>
#include <Support/TestSupport.hpp>

int main()
{
    {
        hsd::stack_array<hsd::usize, 4> arr = {0, 9, 5 ,9};
        auto output = hsd::parse_string<"{},{} -> {},{}", 4>("0,9 -> 5,9").unwrap();

        for (hsd::usize i = 0; i < 4; ++i)
            assert(output[i].to_usize() == arr[i]);

        auto invalid = hsd::parse_string<"{},{} -> {},{}", 4>("0,9 - 5,9");
        assert(!invalid.is_ok());
    }

    {
        auto output = hsd::parse_string<
            "Name: {}\nAge: {}\nColor: {}\n", 3>(
                "Name: Rufus\nAge: 42\nColor: red\n"
            ).unwrap();

        test_support::expect_string(output[0].to_string(), "Rufus");
        assert(output[1].to_i32() == 42);
        test_support::expect_string(output[2].to_string(), "red");
    }

    {
        auto output = hsd::parse_string<"{}:{}:{}:{}", 4>(
            "255:-128:1.5:tail").unwrap();
        assert(output[0].to_u8() == 255);
        assert(output[1].to_i8() == -128);
        assert(output[2].to_f32() == 1.5f);
        assert(output[3].to_string_view().length() == 4);
        assert(output[3].to_string_view()[0] == 't');
        assert(output[3].to_string_view()[3] == 'l');

        auto invalid_pattern = hsd::parse_string<"expected:{}", 1>("actual:5");
        assert(!invalid_pattern.is_ok());
    }

    return 0;
}