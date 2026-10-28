#include "Base/Types.hpp"
#include <Serialize/Io.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/StringParser.hpp>
#include <Containers/Range.hpp>

using tree_type = hsd::vector<hsd::vector<hsd::vector<hsd::isize>>>;

static inline auto parse_line(const hsd::string& str)
{
    hsd::vector<hsd::isize> vec{}; 

    auto res = hsd::parse_string<"{} {}", 2>(str.c_str());
    hsd::string_view view = "";

    while (res.is_ok() == true)
    {
        auto vals = res.unwrap();
        vec.emplace_back(vals[0].to_isize());

        view = vals[1].to_string_view();
        res = hsd::parse_string<"{} {}", 2>(view.data());
    }

    auto res2 = hsd::parse_string<"{}", 1>(view.data());

    if (res2.is_ok() == true)
    {
        auto vals = res2.unwrap();
        vec.emplace_back(vals[0].to_isize());
    }

    return vec;
}

static inline auto read_file()
{
    tree_type tree{};
    auto file = hsd::io::load_file("DayNine.txt").unwrap();
    
    file.get_stream().reserve(1000);
    auto str = file.read_line().unwrap();

    while (file.is_eof() == false)
    {
        tree.emplace_back();
        tree.back().emplace_back();

        tree.back().back() = parse_line(str);
        str = file.read_line().unwrap();
    }

    return tree;
}

static inline void fill_matrix(auto& mat)
{
    for (hsd::usize idx_mat = 0; idx_mat != mat.size(); ++idx_mat)
    {
        if (mat[idx_mat].back() != 0)
        {
            mat.emplace_back();

            auto curr_val = mat[idx_mat][0];
            auto next_val = mat[idx_mat][1];
            mat.back().emplace_back(next_val - curr_val);

            for (hsd::usize idx_vec = 2; idx_vec != mat[idx_mat].size(); ++idx_vec)
            {
                curr_val = next_val;
                next_val = mat[idx_mat][idx_vec];
                mat.back().emplace_back(next_val - curr_val);
            }
        }
    }
}

static inline hsd::isize part1(const tree_type& tree)
{
    hsd::isize total = 0;

    for (const auto& mat : tree)
    {
        hsd::isize prev = 0;

        for (const auto& vec : mat | hsd::views::reverse)
        {
            if (vec.back() != 0)
                prev += vec.back();
        }

        total += prev;
    }

    return total;
}

static inline hsd::isize part2(const tree_type& tree)
{
    hsd::isize total = 0;

    for (const auto& mat : tree)
    {
        hsd::isize prev = 0;

        for (const auto& vec : mat | hsd::views::reverse)
        {
            if (vec.back() != 0)
                prev = vec.front() - prev;
        }

        total += prev;
    }

    return total;
}

int main()
{
    using namespace hsd::format_literals;
        
    auto tree = read_file();
    
    for (auto& mat : tree)
    {
        fill_matrix(mat);
    }
    
    hsd::println("Day 9 part 1 is: {}"_fmt, part1(tree));
    hsd::println("Day 9 part 2 is: {}"_fmt, part2(tree));
}