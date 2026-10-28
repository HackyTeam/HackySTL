#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/StringParser.hpp>

using cubes_type = hsd::unordered_map<hsd::string, hsd::usize>;

enum class possibility_type
{
    not_matches,
    matches_not_valid,
    matches_valid
};

template < hsd::basic_string_literal fmt, hsd::usize N >
static inline possibility_type check_possibility(const char* game, const cubes_type& cubes)
{
    auto possibility = hsd::parse_string<fmt, N>(game);
        
    if (possibility.is_ok())
    {
        auto res = possibility.unwrap();

        for (hsd::usize idx = 0; idx < res.size(); idx += 2)
        {
            if (res[idx].to_usize() > cubes[res[idx + 1].to_string()])
            {
                return possibility_type::matches_not_valid;
            }
        }

        return possibility_type::matches_valid;
    }

    return possibility_type::not_matches;
}

static inline bool check_games(const hsd::string& games, const cubes_type& cubes)
{
    hsd::usize last_pos = 0, next_pos = games.find(';');
    hsd::string game = ""; possibility_type type;
    
    while (next_pos != hsd::string::npos)
    {
        game = hsd::string{games.c_str() + last_pos, next_pos - last_pos + 1};
        type = check_possibility<"{} {}, {} {}, {} {};", 6>(game.c_str(), cubes);

        if (type == possibility_type::matches_not_valid)
        {
            return false;
        }
        else if (type == possibility_type::matches_valid)
        {
            goto next_game;
        }

        type = check_possibility<"{} {}, {} {};", 4>(game.c_str(), cubes);

        if (type == possibility_type::matches_not_valid)
        {
            return false;
        }
        else if (type == possibility_type::matches_valid)
        {
            goto next_game;
        }

        type = check_possibility<"{} {};", 2>(game.c_str(), cubes);

        if (type == possibility_type::matches_not_valid)
        {
            return false;
        }

        // Cursed code ahead:
        next_game:
            last_pos = next_pos + 2;
            next_pos = games.find(';', last_pos);
    }

    game = hsd::string{games.c_str() + last_pos,games.size() - last_pos + 1};
    type = check_possibility<"{} {}, {} {}, {} {}\n", 6>(game.c_str(), cubes);

    if (type == possibility_type::matches_not_valid)
    {
        return false;
    }
    else if (type == possibility_type::matches_valid)
    {
        return true;
    }

    type = check_possibility<"{} {}, {} {}\n", 4>(game.c_str(), cubes);

    if (type == possibility_type::matches_not_valid)
    {
        return false;
    }
    else if (type == possibility_type::matches_valid)
    {
        return true;
    }

    type = check_possibility<"{} {}\n", 2>(game.c_str(), cubes);

    if (type == possibility_type::matches_not_valid)
    {
        return false;
    }

    return true;
}

static inline auto parse_line1(const hsd::string& str, const cubes_type& cubes)
{
    auto game_info = hsd::parse_string<"Game {}: {}\n", 2>(str.c_str()).unwrap();
    auto game_id = game_info[0].to_usize();
    return hsd::pair{check_games(game_info[1].to_string(), cubes), game_id};
}

template < hsd::basic_string_literal fmt, hsd::usize N >
static inline bool set_max_game(const char* game, cubes_type& cubes)
{
    auto possibility = hsd::parse_string<fmt, N>(game);
        
    if (possibility.is_ok())
    {
        auto res = possibility.unwrap();

        for (hsd::usize idx = 0; idx < res.size(); idx += 2)
        {
            if (res[idx].to_usize() > cubes[res[idx + 1].to_string()])
            {
                cubes[res[idx + 1].to_string()] = res[idx].to_usize();
            }
        }

        return true;
    }

    return false;
}

static inline void set_max_games(const hsd::string& games, cubes_type& cubes)
{
    hsd::usize last_pos = 0, next_pos = games.find(';');
    hsd::string game = "";
    
    while (next_pos != hsd::string::npos)
    {
        game = hsd::string{games.c_str() + last_pos, next_pos - last_pos + 1};

        if (set_max_game<"{} {}, {} {}, {} {};", 6>(game.c_str(), cubes))
        {
            goto next_game;
        }

        if (set_max_game<"{} {}, {} {};", 4>(game.c_str(), cubes))
        {
            goto next_game;
        }

        if (set_max_game<"{} {};", 2>(game.c_str(), cubes))
        {
            goto next_game;
        }

        // Cursed code ahead:
        next_game:
            last_pos = next_pos + 2;
            next_pos = games.find(';', last_pos);
    }

    game = hsd::string{games.c_str() + last_pos,games.size() - last_pos + 1};
    
    if (set_max_game<"{} {}, {} {}, {} {}\n", 6>(game.c_str(), cubes))
    {
        return;
    }
    
    if (set_max_game<"{} {}, {} {}\n", 4>(game.c_str(), cubes))
    {
        return;
    }
    
    if (set_max_game<"{} {}\n", 2>(game.c_str(), cubes))
    {
        return;
    }
}

static inline auto parse_line2(const hsd::string& str, cubes_type& cubes)
{
    auto game_info = hsd::parse_string<"Game {}: {}\n", 2>(str.c_str()).unwrap();
    //auto game_id = game_info[0].to_usize();
    set_max_games(game_info[1].to_string(), cubes);

    hsd::usize total = 1;

    for (const auto& [_, val] : cubes)
    {
        total *= val;
    }

    return total;
}

int main()
{
    using namespace hsd::string_literals;
    using namespace hsd::format_literals;
    
    {
        hsd::usize total = 0;

        const cubes_type cubes = {{
            {"red"_s, 12ull},
            {"green"_s, 13ull},
            {"blue"_s, 14ull}
        }};

        auto file = hsd::io::load_file("DayTwo.txt").unwrap();
        file.get_stream().reserve(1000);
        auto str = file.read_line().unwrap();

        while (str.size() > 1 && str[0] != '\n')
        {
            auto&& [valid, id] = parse_line1(str, cubes);

            if (valid == true) total += id;

            str = file.read_line().unwrap();
        }

        hsd::println("Result 1 is: {}"_fmt, total);
    }

    {
        hsd::usize total = 0;

        auto file = hsd::io::load_file("DayTwo.txt").unwrap();
        file.get_stream().reserve(1000);
        auto str = file.read_line().unwrap();

        while (str.size() > 1 && str[0] != '\n')
        {
            cubes_type cubes = {{
                {"red"_s, 0ull},
                {"green"_s, 0ull},
                {"blue"_s, 0ull}
            }};

            auto res = parse_line2(str, cubes);

            total += res;

            str = file.read_line().unwrap();
        }

        hsd::println("Result 2 is: {}"_fmt, total);
    }
}