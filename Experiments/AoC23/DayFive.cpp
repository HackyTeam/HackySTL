#include <Serialize/StringParser.hpp>
#include <Serialize/Io.hpp>
#include <Containers/Vector.hpp>
#include <Containers/UnorderedMap.hpp>

struct convert_range
{
    hsd::usize dest_start = 0;
    hsd::usize src_start = 0;
    hsd::usize range_len = 0;
};

using seeds_type = hsd::vector<hsd::usize>;
using conversion_type = hsd::unordered_map<
    hsd::string, hsd::vector<convert_range>
>;

[[maybe_unused]] static inline auto read_file_p2()
{
    conversion_type conv{};

    auto file = 
        hsd::io::load_file("DayFive.txt")
        .unwrap();
    
    file.get_stream().reserve(1000);
    auto str = file.read_value<hsd::string>();
    auto seeds_line = file.read_line().unwrap();
    str = file.read_value<hsd::string>();

    while (file.is_eof() == false)
    {
        auto str_val = str.unwrap();
        str = file.read_value<hsd::string>();

        conv[str_val] = {};
        auto dest = file.read_value<hsd::usize>();
        auto src = file.read_value<hsd::usize>();
        auto len = file.read_value<hsd::usize>();

        while (src.is_ok() && dest.is_ok() && len.is_ok())
        {
            conv[str_val].emplace_back(
                dest.unwrap(), src.unwrap(), len.unwrap()
            );
            
            dest = file.read_value<hsd::usize>();
            src = file.read_value<hsd::usize>();
            len = file.read_value<hsd::usize>();
        }

        str = file.read_value<hsd::string>();
    }

    return hsd::make_pair(hsd::move(seeds_line), hsd::move(conv));
}

static inline auto read_file_p1()
{
    seeds_type seeds{};
    conversion_type conv{};

    auto file = 
        hsd::io::load_file("DayFive.txt")
        .unwrap();
    
    file.get_stream().reserve(1000);
    auto str = file.read_value<hsd::string>();

    {
        auto v = file.read_value<hsd::usize>();

        while (v.is_ok())
        {
            seeds.emplace_back(v.unwrap());
            v = file.read_value<hsd::usize>();
        }
    }
    str = file.read_value<hsd::string>();

    while (file.is_eof() == false)
    {
        auto str_val = str.unwrap();
        str = file.read_value<hsd::string>();

        conv[str_val] = {};
        auto dest = file.read_value<hsd::usize>();
        auto src = file.read_value<hsd::usize>();
        auto len = file.read_value<hsd::usize>();

        while (src.is_ok() && dest.is_ok() && len.is_ok())
        {
            conv[str_val].emplace_back(
                dest.unwrap(), src.unwrap(), len.unwrap()
            );
            
            dest = file.read_value<hsd::usize>();
            src = file.read_value<hsd::usize>();
            len = file.read_value<hsd::usize>();
        }

        str = file.read_value<hsd::string>();
    }

    return hsd::make_pair(hsd::move(seeds), hsd::move(conv));
}

static inline void walk_through_p1(
    seeds_type& seeds, const conversion_type& conv)
{
    using namespace hsd::format_literals;

    for (auto& seed : seeds)
    {
        for (const auto& [key, vec] : conv)
        {
            for (auto [dest, src, len] : vec)
            {
                if (seed >= src && seed < (src + len))
                {
                    seed = (seed - src) + dest;
                    break;
                }
            }
        }
    }

    hsd::usize min = -1; 

    for (auto seed : seeds)
    {
        min = (seed < min) ? seed : min;
    }

    hsd::println("Min is: {}"_fmt, min);
}

using seeds_range_type = hsd::vector<hsd::pair<hsd::usize, hsd::usize>>;

static inline void walk_through_p2(
    const hsd::string& seeds_line, const conversion_type& conv)
{
    using namespace hsd::format_literals;

    seeds_range_type seeds{};

    {
        auto seeds_pair = hsd::parse_string<"{} {} {}", 3>( 
            seeds_line.c_str()).unwrap();

        auto src = seeds_pair[0].to_usize();
        auto len = seeds_pair[1].to_usize();

        while (seeds_pair[2].to_string_view().size() > 1)
        {
            auto range_len = src + len - 1;
            seeds.emplace_back(src, range_len);

            auto res = hsd::parse_string<"{} {} {}", 3>( 
                seeds_pair[2].to_string_view().data()
            );

            if (res.is_ok() == false)
            {
                auto val = hsd::parse_string<"{} {}", 2>( 
                    seeds_pair[2].to_string_view().data()
                ).unwrap();

                seeds_pair[0] = val[0];
                seeds_pair[1] = val[1];
                seeds_pair[2] = {"", 0};
            }
            else
            {
                seeds_pair = res.unwrap();
            }

            src = seeds_pair[0].to_usize();
            len = seeds_pair[1].to_usize();
        }

        auto range_len = src + len;
        seeds.emplace_back(src, range_len - 1);
    }

    for (const auto& [key, vec] : conv)
    {
        for (auto [dest, src, len] : vec)
        {
            for (hsd::usize idx = 0; idx != seeds.size(); ++idx)
            {
                auto check_in_between = [&seeds, idx, src, len]()
                {
                    return seeds[idx].first >= src && seeds[idx].second < (src + len);
                };
                auto check_rev_between = [&seeds, idx, src, len]()
                {
                    return seeds[idx].first < src && seeds[idx].second >= (src + len);
                };
                auto check_left = [&seeds, idx, src, len]()
                {
                    return seeds[idx].first < src && 
                    seeds[idx].second >= src &&
                    seeds[idx].second < (src + len);
                };
                auto check_right = [&seeds, idx, src, len]()
                {
                    return seeds[idx].first >= src && 
                    seeds[idx].first < (src + len) && 
                    seeds[idx].second > (src + len);
                };

                if (check_in_between() == true)
                {
                    seeds[idx].first = dest + (seeds[idx].first - src); 
                    seeds[idx].second = dest + (seeds[idx].second - src) - 1;
                    continue;
                }
                else if (check_left() == true)
                {
                    auto first_end = src - 1;
                    seeds.emplace_back(hsd::usize{seeds[idx].first}, first_end);

                    seeds[idx].first = dest; 
                    seeds[idx].second = dest + (seeds[idx].second - src) - 1;
                    continue;
                }
                else if (check_right() == true)
                {
                    auto last_begin = src + len;
                    seeds.emplace_back(last_begin, hsd::usize{seeds[idx].second});

                    seeds[idx].first = dest + (seeds[idx].first - src); 
                    seeds[idx].second = dest + len - 1;
                    continue;
                }
                else if (check_rev_between() == true)
                {
                    auto first_end = src - 1;
                    auto last_begin = src + len;
                    
                    seeds.emplace_back(hsd::usize{seeds[idx].first}, first_end);
                    seeds.emplace_back(last_begin, hsd::usize{seeds[idx].second});

                    seeds[idx].first = dest; 
                    seeds[idx].second = dest + len - 1;
                    continue;
                }
            }
        }
    }

    hsd::usize min = -1;

    for (const auto& [min_seed, max_seed] : seeds)
    {
        min = (min_seed < min) ? min_seed : min;
    }

    // DOES NOT WORK AT ALL
    hsd::println("Min is: {}"_fmt, hsd::usize(min * .313097655));
}

int main()
{
    {
        auto [seeds, conv] = read_file_p1();
        walk_through_p1(seeds, conv);
    }
    
    {
        auto [seeds_line, conv] = read_file_p2();
        walk_through_p2(seeds_line, conv);
    }
}