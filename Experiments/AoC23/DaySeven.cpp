#include <Base/Algorithm.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Vector.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>

struct cards_type
{
    hsd::string cards = "";
    hsd::usize amount = 0;
};

using deck_type = hsd::vector<cards_type>;
using game_type = hsd::unordered_map<hsd::string_view, deck_type>;

static inline auto read_file()
{
    using namespace hsd::string_view_literals;

    hsd::usize total = 0;
    game_type game{{
        hsd::make_pair("Five of a kind"_sv, deck_type{}),
        hsd::make_pair("Four of a kind"_sv, deck_type{}),
        hsd::make_pair("Full house"_sv, deck_type{}),
        hsd::make_pair("Three of a kind"_sv, deck_type{}),
        hsd::make_pair("Two pair"_sv, deck_type{}),
        hsd::make_pair("One pair"_sv, deck_type{}),
        hsd::make_pair("High card"_sv, deck_type{})
    }};

    const hsd::unordered_map card_map = {{
        hsd::make_pair('J', 1),
        hsd::make_pair('2', 2),
        hsd::make_pair('3', 3),
        hsd::make_pair('4', 4),
        hsd::make_pair('5', 5),
        hsd::make_pair('6', 6),
        hsd::make_pair('7', 7),
        hsd::make_pair('8', 8),
        hsd::make_pair('9', 9),
        hsd::make_pair('T', 10),
        hsd::make_pair('Q', 11),
        hsd::make_pair('K', 12),
        hsd::make_pair('A', 13)
    }};

    hsd::stack_array<hsd::usize, 13> arr{};
    hsd::vector<hsd::usize> j_indices{};

    auto file = hsd::io::load_file("DaySeven.txt").unwrap();
    file.get_stream().reserve(1000);
    
    auto cards = file.read_value<hsd::string>().unwrap_or_default();
    auto amount = file.read_value<hsd::usize>().unwrap_or_default();

    while (cards.size() > 1 && cards[0] != '\n')
    {
        hsd::usize max_val = 2;
        ++total; arr.fill(0); j_indices.clear();

        for (hsd::usize idx = 0; char& chr : cards)
        {
            if (chr == 'J') j_indices.emplace_back(idx);

            auto v = card_map[chr];
            ++arr[v - 1]; chr = v; ++idx;


            max_val = (v != 1 && arr[v - 1] > arr[max_val - 1]) ? v : max_val; 
        }

        for (auto idx : j_indices)
        {
            --arr[0]; ++arr[max_val - 1];
            cards[idx] = max_val;
        }

        bool is_high = true; 
        bool maybe_fh = false;
        bool maybe_tp = false;
        bool is_fh = false;
        bool is_tp = false;

        for (auto val : arr)
        {
            if (val == 5)
            {
                for (auto idx : j_indices)
                {
                    cards[idx] = 1;
                }

                game["Five of a kind"_sv].emplace_back(cards, amount);
                is_high = false;
                break;
            }
            if (val == 4)
            {
                for (auto idx : j_indices)
                {
                    cards[idx] = 1;
                }

                game["Four of a kind"_sv].emplace_back(cards, amount);
                is_high = false;
                break;
            }
            if (val == 3)
            {
                if (maybe_fh == true)
                {
                    for (auto idx : j_indices)
                    {
                        cards[idx] = 1;
                    }

                    game["Full house"_sv].emplace_back(cards, amount);
                    is_fh = true;
                    break;
                }

                maybe_fh = true;
                is_high = false;
            }
            if (val == 2)
            {
                if (maybe_tp == true)
                {
                    for (auto idx : j_indices)
                    {
                        cards[idx] = 1;
                    }

                    game["Two pair"_sv].emplace_back(cards, amount);
                    is_tp = true;
                    break;
                }
                else if (maybe_fh == true)
                {
                    for (auto idx : j_indices)
                    {
                        cards[idx] = 1;
                    }

                    game["Full house"_sv].emplace_back(cards, amount);
                    is_fh = true;
                    break;
                }
                else
                {
                    maybe_tp = maybe_fh = true;
                    is_high = false;
                }
            }
        }

        if (is_high == false && maybe_fh == true && maybe_tp == true && is_fh == false && is_tp == false)
        {
            for (auto idx : j_indices)
            {
                cards[idx] = 1;
            }

            game["One pair"_sv].emplace_back(cards, amount);
        }
        else if (is_high == false && maybe_fh == true && is_fh == false && is_tp == false)
        {
            for (auto idx : j_indices)
            {
                cards[idx] = 1;
            }

            game["Three of a kind"_sv].emplace_back(cards, amount);
        }
        else if (is_high == true && is_fh == false && is_tp == false)
        {
            for (auto idx : j_indices)
            {
                cards[idx] = 1;
            }

            game["High card"_sv].emplace_back(cards, amount);
        }
        
        cards = file.read_value<hsd::string>().unwrap_or_default();
        amount = file.read_value<hsd::usize>().unwrap_or_default();
    }

    return make_pair(total, hsd::move(game));
}

static inline void do_day7()
{
    hsd::usize total = 0;

    auto [rank_total, game] = read_file();
    
    for (auto& [key, decks] : game)
    {
        if (decks.size() > 1)
        {
            hsd::quick_sort(
                decks, [](auto& low, auto& high)
                {
                    return low.cards < high.cards;
                }
            );
        }

        for (auto& deck : decks)
        {
            total += (rank_total--) * deck.amount;
        }
    }

    using namespace hsd::format_literals;
    hsd::println("Day 7 is: {}"_fmt, total);
}

int main()
{
    do_day7();
}