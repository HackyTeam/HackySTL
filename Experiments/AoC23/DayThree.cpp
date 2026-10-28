#include <Serialize/Io.hpp>
#include <Containers/Vector.hpp>

struct pos_type
{
    hsd::usize x_coord;
    hsd::usize y_coord;

    bool operator==(const pos_type& rhs) const
    {
        return x_coord == rhs.x_coord && y_coord == rhs.y_coord;
    }
};

struct input_value
{
    enum class input_type
    {
        undefined,
        period,
        digit,
        gear
    };

    pos_type symbol_pos = {};
    bool around_gear = false;
    char value = '.';
    input_type type = input_type::period;
};

using line_type = hsd::vector<input_value>;
using input_matrix = hsd::vector<line_type>;
using num_stor_type = hsd::vector<hsd::usize>;
using numbers_type = hsd::vector<hsd::pair<pos_type, num_stor_type>>;

static inline void gen_numbers(const line_type& line, numbers_type& numbers)
{
    using namespace hsd::format_literals;

    char num_str[4] = {}; hsd::usize idx = 0;
    bool is_valid_num = false; pos_type pos{};

    for (const auto& val : line)
    {
        if (val.type == input_value::input_type::digit)
        {
            is_valid_num = val.around_gear ? true : is_valid_num;
            pos = val.around_gear ? val.symbol_pos : pos;
            num_str[idx++] = val.value;
        }
        else if (is_valid_num && val.type != input_value::input_type::digit)
        {
            num_str[idx] = '\0'; idx = 0;
            auto num = hsd::cstring::parse<hsd::usize>(num_str);

            is_valid_num = false;
            
            auto it = hsd::find(
                numbers.begin(), numbers.end(), [pos](const auto& val)
                {
                    return val.first == pos;
                }
            );

            if (it != numbers.end())
            {
                it->second.emplace_back(num);
            }
            else
            {
                numbers.emplace_back(pos, num_stor_type{});
                numbers.back().second.emplace_back(num);
            }
            
            pos = {};
        }
        else
        {
            is_valid_num = false;
            idx = 0; pos = {};
        }
    }
}

static inline void modify_lines(input_matrix& lines)
{
    using enum input_value::input_type;

    for (hsd::usize first = 1; first < lines.size() - 1; ++first)
    {
        for (hsd::usize second = 1; second < lines[first].size() - 1; ++second)
        {
            if (lines[first][second].type == gear)
            {
                hsd::vector<pos_type> pos{};

                if (lines[first - 1][second].type == digit)
                {
                    pos.emplace_back(first - 1, second);
                }
                else
                {
                    if(lines[first - 1][second - 1].type == digit)
                    {
                        pos.emplace_back(first - 1, second - 1);
                    }
                    if(lines[first - 1][second + 1].type == digit)
                    {
                        pos.emplace_back(first - 1, second + 1);
                    }
                }
                
                if (lines[first][second - 1].type == digit)
                {
                    pos.emplace_back(first, second - 1);
                }
                if (lines[first][second + 1].type == digit)
                {
                    pos.emplace_back(first, second + 1);
                }
                
                if (lines[first + 1][second].type == digit)
                {
                    pos.emplace_back(first + 1, second);
                }
                else
                {
                    if(lines[first + 1][second - 1].type == digit)
                    {
                        pos.emplace_back(first + 1, second - 1);
                    }
                    if(lines[first + 1][second + 1].type == digit)
                    {
                        pos.emplace_back(first + 1, second + 1);
                    }
                }

                if (pos.size() == 2)
                {
                    lines[pos[0].x_coord][pos[0].y_coord].around_gear = true;
                    lines[pos[0].x_coord][pos[0].y_coord].symbol_pos = {first, second};
                    lines[pos[1].x_coord][pos[1].y_coord].around_gear = true;
                    lines[pos[1].x_coord][pos[1].y_coord].symbol_pos = {first, second};
                }
            }
        }
    }
}

static inline void use_string(input_matrix& mat, const hsd::string& str)
{
    using enum input_value::input_type;

    mat.emplace_back();
    mat.back().emplace_back();

    for (char val : str)
    {
        input_value::input_type type = undefined;

        if (val == '.')
        {
            type = period;
        }
        else if (val >= '0' && val <= '9')
        {
            type = digit;
        }
        else if (val == '*')
        {
            type = gear;
        }

        mat.back().emplace_back(pos_type{}, false, val, type);
    }

    mat.back().emplace_back();
}

static inline auto read_file()
{
    input_matrix mat{};
    auto file = hsd::io::load_file("DayThree.txt").unwrap();
    
    file.get_stream().reserve(1000);
    auto str = file.read_line().unwrap();

    auto len = str.length() + 2;

    mat.emplace_back();

    for (hsd::usize idx = 0; idx < len; ++idx)
    {
        mat.back().emplace_back();
    }

    while (str.size() > 1 && str[0] != '\n')
    {
        use_string(mat, str);
        str = file.read_line().unwrap();
    }

    mat.emplace_back();

    for (hsd::usize idx = 0; idx < len; ++idx)
    {
        mat.back().emplace_back();
    }

    return mat;
}

int main()
{
    using namespace hsd::format_literals;

    hsd::usize total = 0;
    auto mat = read_file();
    modify_lines(mat);
    numbers_type numbers;

    for (const auto& line : mat)
    {
        gen_numbers(line, numbers);
    }

    for (auto num : numbers)
    {
        total += num.second[0] * num.second[1];
    }

    hsd::println("Total is: {}"_fmt, total);
}