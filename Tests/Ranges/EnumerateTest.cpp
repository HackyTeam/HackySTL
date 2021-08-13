#include <Vector.hpp>
#include <Span.hpp>
#include <Io.hpp>

int main()
{
    hsd::vector<hsd::i32> vec = {{1, 2, 3, 4, 5, 6, 7, 8, 9}};

    for (auto [val, count] : vec.to_span().enumerate())
    {
        hsd_print("({}, {}) ", val, count);
        val = val + 1;
    }

    hsd_println("");

    for (auto& val : vec)
    {
        hsd_print("{} ", val);
    }

    hsd_println("");
}