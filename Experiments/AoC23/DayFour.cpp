#include <Serialize/StringParser.hpp>
#include <Serialize/Io.hpp>

static inline auto separate_string(const hsd::string& string)
{
    hsd::vector<hsd::string> strings;
    auto res = hsd::parse_string<"{} {}", 2>(string.c_str());
    hsd::string_view view = "";

    while (res.is_ok() == true)
    {
        auto vals = res.unwrap();

        if (vals[0].to_string_view().size() != 0)
        {
            strings.emplace_back(vals[0].to_string());
        }

        view = vals[1].to_string_view();
        res = hsd::parse_string<"{} {}", 2>(view.data());
    }

    auto res2 = hsd::parse_string<"{} ", 1>(view.data());

    if (res2.is_ok() == true)
    {
        auto vals = res2.unwrap();

        if (vals[0].to_string_view().size() != 0)
        {
            strings.emplace_back(vals[0].to_string());
        }
    }

    return strings;
}

using game_type_p1 = hsd::pair<hsd::vector<hsd::string>, hsd::vector<hsd::string>>;

static inline auto parse_line_p1(const hsd::string& line)
{
    game_type_p1 result;

    auto line_res = hsd::parse_string<"Card {}: {}| {}", 3>(line.c_str()).unwrap();
    result.first = separate_string(line_res[1].to_string());
    result.second = separate_string(line_res[2].to_string());

    return result;
}

static inline auto read_file_p1()
{
    hsd::vector<game_type_p1> games{};
    auto file = hsd::io::load_file("DayFour.txt").unwrap();
    
    file.get_stream().reserve(1000);
    auto str = file.read_line().unwrap();

    while (str.size() > 1 && str[0] != '\n')
    {
        games.emplace_back(parse_line_p1(str));
        str = file.read_line().unwrap();
    }

    return games;
}

static inline void part1()
{
    using namespace hsd::format_literals;

    hsd::usize total = 0;
    auto games = read_file_p1();

    for (const auto& [winner, game] : games)
    {
        hsd::usize sum = 0;

        for (const auto& first : winner)
        {
            for (const auto& second : game)
            {
                if (first == second) ++sum;
            }
        }

        if (sum > 0) total += 1 << (sum - 1);
    }

    hsd::println("Part 1 is: {}"_fmt, total);
}

struct game_info
{
    hsd::usize num_games = 1;
    hsd::usize game_id;
    hsd::vector<hsd::string> winners;
    hsd::vector<hsd::string> games;
};

struct game_info_view
{
    hsd::usize game_id = 0;
    hsd::usize num_wins = 0;
    hsd::usize num_instances = 1;
};

static inline auto parse_line_p2(const hsd::string& line)
{
    game_info result{};

    auto line_res = hsd::parse_string<"Card {}: {}| {}", 3>(line.c_str()).unwrap();
    result.game_id = line_res[0].to_usize();
    result.games = separate_string(line_res[1].to_string());
    result.winners = separate_string(line_res[2].to_string());

    return result;
}

static inline auto read_file_p2()
{
    hsd::vector<game_info> games{};
    auto file = hsd::io::load_file("DayFour.txt").unwrap();
    
    file.get_stream().reserve(1000);
    auto str = file.read_line().unwrap();

    while (str.size() > 1 && str[0] != '\n')
    {
        games.emplace_back(parse_line_p2(str));
        str = file.read_line().unwrap();
    }

    return games;
}

static inline void part2()
{
    using namespace hsd::format_literals;

    hsd::usize total = 0;
    hsd::vector<game_info_view> games;

    {
        auto games_to_check = read_file_p2();

        for (const auto& game : games_to_check)
        {
            hsd::usize sum = 0;

            for (const auto& first : game.winners)
            {
                for (const auto& second : game.games)
                {
                    if (first == second) ++sum;
                }
            }

            games.emplace_back(game.game_id, sum);
        }
    }

    for (hsd::usize game_idx = 0; game_idx != games.size(); ++game_idx)
    {
        auto curr_game_id = games[game_idx].game_id;

        for (hsd::usize idx = 0; idx < games[game_idx].num_wins && curr_game_id + idx < games.size(); ++idx)
        {
            games[curr_game_id + idx].num_instances += games[game_idx].num_instances;
        }
    }

    for (const auto& game : games)
    {
        total += game.num_instances;
    }

    hsd::println("Part 2 is: {}"_fmt, total);
}

int main()
{
    part1();
    part2();
}