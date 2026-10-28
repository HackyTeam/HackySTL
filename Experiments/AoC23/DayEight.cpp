#include <Serialize/StringParser.hpp>
#include <Serialize/Io.hpp>
#include <Containers/Vector.hpp>
#include <Containers/UnorderedMap.hpp>

using graph_type = hsd::pair<hsd::string, hsd::string>;
using graph_map = hsd::unordered_map<hsd::string, graph_type>;

static inline auto read_file()
{
    graph_map map{};
    auto file = hsd::io::load_file("DayEight.txt").unwrap();
    
    file.get_stream().reserve(1000);
    auto directions = file.read_line().unwrap();
    auto str = file.read_line().unwrap();

    while (str.size() > 1 && str[0] != '\n')
    {
        auto graph = hsd::parse_string<"{} = ({}, {})", 3>(str.c_str()).unwrap();
        
        map[graph[0].to_string()] = {
            graph[1].to_string(), graph[2].to_string()
        };

        str = file.read_line().unwrap();
    }

    return hsd::make_pair(hsd::move(directions), hsd::move(map));
}

int main()
{
    {
        using namespace hsd::format_literals;
        using namespace hsd::string_view_literals;

        hsd::string current = "AAA";
        auto [directions, map] = read_file();
        hsd::usize total = 0;

        while (current != "ZZZ"_sv)
        {
            auto idx = total % directions.length();
            ++total;

            if (directions[idx] == 'L')
            {
                current = map[current].first;
            }
            else
            {
                current = map[current].second;
            }
        }

        hsd::println("Day 8 Part 1 is: {}"_fmt, total);
    }

    {
        using namespace hsd::format_literals;
        using namespace hsd::string_view_literals;

        hsd::vector<hsd::string> current = {};
        auto [directions, map] = read_file();

        for (const auto& [key, _] : map)
        {
            if (key[2] == 'A')
            {
                current.emplace_back(key);
            }
        }

        hsd::print("Day 8 Part 2 is: "_fmt);

        for (auto& val : current)
        {
            hsd::usize total = 0;

            while (val[2] != 'Z')
            {
                auto idx = total % directions.length();
                ++total;
    
                bool is_right = (directions[idx] == 'L');
                const auto& [right, left] = map[val];
                
                for (hsd::usize str_idx = 0; char chr : (is_right) ? right : left)
                {
                    val[str_idx] = chr;
                    ++str_idx;
                }
            }

            hsd::print("{} "_fmt, total);
        }
        hsd::print("\n"_fmt);
    }
}