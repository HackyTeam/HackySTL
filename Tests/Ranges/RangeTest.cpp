#include <Range.hpp>
#include <Vector.hpp>
#include <List.hpp>
#include <UnorderedMap.hpp>
#include <Io.hpp>

int main()
{
    {
        puts("hsd::vector:");
        hsd::vector vals = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};

        for(auto& val : vals)
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::drop(3))
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::reverse)
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::reverse | hsd::views::drop(5))
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::random)
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::random | hsd::views::drop(2))
            hsd::io::print<"{} ">(val);

        puts("");
    }

    {
        puts("hsd::list:");
        hsd::list vals = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};

        for(auto& val : vals)
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::drop(3))
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::reverse)
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::reverse | hsd::views::drop(5))
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::random)
            hsd::io::print<"{} ">(val);

        puts("");

        for(auto& val : vals | hsd::views::random | hsd::views::drop(2))
            hsd::io::print<"{} ">(val);

        puts("");
    }

    {
        puts("hsd::unordered_map:");
        using namespace hsd::string_literals;
        hsd::unordered_map<hsd::string_view, hsd::i32> vals = {{
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

        for(auto& val : vals)
            hsd::io::print<"({}, {}) ">(val.first, val.second);

        puts("");

        for(auto& val : vals | hsd::views::drop(3))
            hsd::io::print<"({}, {}) ">(val.first, val.second);

        puts("");

        for(auto& val : vals | hsd::views::reverse)
            hsd::io::print<"({}, {}) ">(val.first, val.second);

        puts("");

        for(auto& val : vals | hsd::views::reverse | hsd::views::drop(5))
            hsd::io::print<"({}, {}) ">(val.first, val.second);

        puts("");

        for(auto& val : vals | hsd::views::random)
            hsd::io::print<"({}, {}) ">(val.first, val.second);

        puts("");

        for(auto& val : vals | hsd::views::random | hsd::views::drop(2))
            hsd::io::print<"({}, {}) ">(val.first, val.second);

        puts("");
    }
}