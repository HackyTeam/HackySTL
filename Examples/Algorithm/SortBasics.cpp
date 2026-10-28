// Title: Sorting basics
// Level: Basic
// Shows: quick_sort with the default order, a custom predicate, and strings.

#include <Base/Algorithm.hpp>
#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename Vec>
void print_all(const char* label, const Vec& values)
{
    hsd::print("{}:"_fmt, label);
    for (const auto& value : values)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
}

int main()
{
    hsd::vector<hsd::i32> numbers{{5, 2, 9, 1, 7}};

    // The default predicate orders the smallest value first.
    hsd::quick_sort(numbers);
    print_all("ascending", numbers);

    // A predicate returning true places its first argument after the second,
    // so comparing with '<' gives the reverse order.
    hsd::quick_sort(numbers, [](hsd::i32 lhs, hsd::i32 rhs) { return lhs < rhs; });
    print_all("descending", numbers);

    hsd::vector<hsd::string> words{{
        hsd::string{"pear"}, hsd::string{"apple"}, hsd::string{"fig"}
    }};
    hsd::quick_sort(words);
    print_all("words", words);

    // A predicate orders records that have no operator>.
    struct edge_record
    {
        hsd::i32 key;
        hsd::i32 id;
    };

    hsd::vector<edge_record> edge_records{{edge_record{3, 0}, edge_record{1, 1}, edge_record{2, 2}}};
    hsd::quick_sort(edge_records, [](const edge_record& lhs, const edge_record& rhs)
    {
        return lhs.key > rhs.key;
    });
    hsd::print("records by key:"_fmt);
    for (const auto& record : edge_records)
    {
        hsd::print(" {}"_fmt, record.key);
    }
    hsd::println(""_fmt);
    return 0;
}
