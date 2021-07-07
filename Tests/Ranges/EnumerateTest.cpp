#include <Vector.hpp>
#include <Span.hpp>
#include <Io.hpp>

int main()
{
    hsd::vector<hsd::i32> vec = {{1, 2, 3, 4, 5, 6, 7, 8, 9}};

    for (auto [val, count] : vec.to_span().enumerate())
    {
        hsd::io::print<"({}, {}) ">(val, count);
        val = val + 1;
    }

    hsd::io::print<"\n">();

    for (auto& val : vec)
    {
        hsd::io::print<"{} ">(val);
    }

    hsd::io::print<"\n">();
}