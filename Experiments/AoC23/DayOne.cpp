#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

static inline auto synth_digit_str(const hsd::string& val)
{
    using namespace hsd::string_literals;

    const hsd::stack_array digits = {
        "zero"_s, "one"_s, 
        "two"_s, "three"_s, 
        "four"_s, "five"_s, 
        "six"_s, "seven"_s, 
        "eight"_s, "nine"_s, 
    };

    hsd::string new_str;

    for (hsd::usize idx = 0; idx < val.size(); ++idx)
    {
        hsd::usize arr_idx, found_pos = -1;

        for (arr_idx = 0; arr_idx < digits.size() && found_pos != idx; ++arr_idx)
        {
            found_pos = val.find(digits[arr_idx], idx);
        }

        if (found_pos == idx)
        {
            new_str.emplace_back(static_cast<char>('0' + arr_idx - 1ul));
        }
        else
        {
            new_str.emplace_back(val[idx]);
        }
    }

    return new_str;
};

int main()
{
    using namespace hsd::format_literals;
    
    hsd::i32 first = -1, second = -1, total = 0;
    auto file = hsd::io::load_file("DayOne.txt").unwrap();
    
    file.get_stream().reserve(100);
    auto str = file.read_line().unwrap();
    
    hsd::string copy;

    while (str.size() > 1 && str[0] != '\n')
    {
        first = second = -1;
        copy = synth_digit_str(str);

        for (char chr : copy)
        {
            hsd::i32 val = chr - '0';

            if (val >= 0 && val <= 9)
            {
                if (first == -1)
                {
                    first = val;
                }

                second = val;
            }
        }
        
        first = (first != -1) ? first : 0;
        second = (second != -1) ? second : 0;

        total += (first * 10 + second);
        copy = str;
        str = file.read_line().unwrap();
    }

    hsd::println("Result is: {}"_fmt, total);
}