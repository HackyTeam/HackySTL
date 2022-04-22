#include <Range.hpp>
#include <Vector.hpp>
#include <List.hpp>
#include <UnorderedMap.hpp>
#include <Io.hpp>
#include <StringView.hpp>

int main()
{
    using namespace hsd::format_literals;

    {
        hsd::println("hsd::vector:"_fmt);

        constexpr auto is_even = [](const auto& val){ return (val & 1) == 0; };
        constexpr auto square = [](const auto& val){ return val * val; };
        constexpr auto less = [](const auto& val){ return val < 5; };
        constexpr auto is_even_square = [](const auto& val) 
            -> hsd::optional<hsd::remove_cvref_t<decltype(val)>>
        {
            if ((val & 1) == 0)
            {
                return val * val;
            }
            else
            {
                return {};
            }
        };

        hsd::vector vals = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};

        hsd::println("Forward:"_fmt);

        for (auto& val : vals)
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nForward, dropped 3 elements:"_fmt);

        for (auto& val : vals | hsd::views::drop(3))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nForward, dropped until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::drop_while(less))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nForward, took 3 elements:"_fmt);

        for (auto& val : vals | hsd::views::take(3))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nForward, took until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::take_while(less))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nForward, filtred:"_fmt);

        for (auto& val : vals | hsd::ranges::filter(is_even))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nForward, transformed:"_fmt);

        for (auto& val : vals | hsd::ranges::transform(square))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nForward, filtered and transformed:"_fmt);

        for (auto& val : vals | hsd::ranges::filter_map(is_even_square))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed:"_fmt);

        for (auto& val : vals | hsd::views::reverse)
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed, dropped 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::views::drop(5))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed, dropped until 5 elements:"_fmt);

        for(auto& val : vals | hsd::views::reverse | hsd::views::drop_while(less))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed, took 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::views::take(5))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed, took until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::views::take_while(less))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed, filtred:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::ranges::filter(is_even))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed, transformed:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::ranges::transform(square))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nReversed, filtered and transformed:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::ranges::filter_map(is_even_square))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom:"_fmt);

        for(auto& val : vals | hsd::views::random)
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom, dropped 2 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::drop(2))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom, dropped until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::drop_while(less))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom, took 2 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::take(2))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom, took until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::take_while(less))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom, filtred:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::ranges::filter(is_even))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom, transformed:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::ranges::transform(square))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::println("\nRandom, filtred and transformed:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::ranges::filter_map(is_even_square))
        {
            hsd::print("{} "_fmt, val);
        }

        hsd::print("\n"_fmt);
    }

    {
        hsd::println("hsd::unordered_map:"_fmt);
        
        using namespace hsd::string_view_literals;
        constexpr auto less = [](const auto& val){ return val.second < 5; };
        constexpr auto is_even = [](const auto& val){ return (val.second & 1) == 0; };
        constexpr auto square = [](const auto& val){ return hsd::pair{val.first, val.second * val.second}; };
        constexpr auto is_even_square = [](const auto& val) 
            -> hsd::optional<hsd::remove_cvref_t<decltype(val)>>
        {
            if ((val.second & 1) == 0)
            {
                return hsd::pair{val.first, val.second * val.second};
            }
            else
            {
                return {};
            }
        };

        hsd::unordered_map vals = {{
            hsd::pair{"first"_sv, 1}, 
            hsd::pair{"second"_sv, 2}, 
            hsd::pair{"third"_sv, 3}, 
            hsd::pair{"forth"_sv, 4}, 
            hsd::pair{"fifth"_sv, 5}, 
            hsd::pair{"sixth"_sv, 6}, 
            hsd::pair{"sevnth"_sv, 7}, 
            hsd::pair{"eighth"_sv, 8}, 
            hsd::pair{"ninth"_sv, 9}, 
            hsd::pair{"tenth"_sv, 10}
        }};

        hsd::println("Forward:"_fmt);

        for (auto& val : vals)
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nForward, dropped 3 elements:"_fmt);

        for (auto& val : vals | hsd::views::drop(3))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nForward, dropped until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::drop_while(less))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nForward, took 3 elements:"_fmt);

        for (auto& val : vals | hsd::views::take(3))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nForward, took until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::take_while(less))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nForward, filtred:"_fmt);

        for (auto& val : vals | hsd::ranges::filter(is_even))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nForward, transformed:"_fmt);

        for (auto& val : vals | hsd::ranges::transform(square))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nForward, filtered and transformed:"_fmt);

        for (auto& val : vals | hsd::ranges::filter_map(is_even_square))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed:"_fmt);

        for (auto& val : vals | hsd::views::reverse)
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed, dropped 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::views::drop(5))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed, dropped until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::views::drop_while(less))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed, took 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::views::take(5))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed, took until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::views::take_while(less))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed, filtred:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::ranges::filter(is_even))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed, transformed:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::ranges::transform(square))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nReversed, filtered and transformed:"_fmt);

        for (auto& val : vals | hsd::views::reverse | hsd::ranges::filter_map(is_even_square))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom:"_fmt);

        for (auto& val : vals | hsd::views::random)
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom, dropped 2 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::drop(2))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom, dropped until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::drop_while(less))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom, took 2 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::take(2))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom, took until 5 elements:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::views::take_while(less))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom, filtred:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::ranges::filter(is_even))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom, transformed:"_fmt);

        for (auto& val : vals | hsd::views::random | hsd::ranges::transform(square))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::println("\nRandom, filtered and transformed:"_fmt);
 
        for (auto& val : vals | hsd::views::random | hsd::ranges::filter_map(is_even_square))
        {
            hsd::print("({}, {}) "_fmt, val.first, val.second);
        }

        hsd::print("\n"_fmt);
    }
}