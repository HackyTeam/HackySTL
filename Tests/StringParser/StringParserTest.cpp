#include <StringParser.hpp>
#include <Io.hpp>

int main()
{
    {
        hsd::stack_array<hsd::usize, 4> arr = {0, 9, 5 ,9};
        auto output = hsd::parse_string<"{},{} -> {},{}", 4>("0,9 -> 5,9").unwrap();

        for (hsd::usize i = 0; i < 4; ++i)
        {
            if (output[i].to_usize() != arr[i])
            {
                hsd::panic("Failed to parse string");
            }
        }
    }

    {
        auto output = hsd::parse_string<
            "Name: {}\nAge: {}\nColor: {}\n", 3>(
                "Name: Rufus\nAge: 42\nColor: red\n"
            ).unwrap();

        hsd_println(
            "({}, {}, {})", 
            output[0].to_string(), 
            output[1].to_string(), 
            output[2].to_string()
        );
    }

    return 0;
}