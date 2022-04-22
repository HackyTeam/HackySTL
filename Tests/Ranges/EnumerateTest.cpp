#include <Vector.hpp>
#include <Span.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::format_literals;

    hsd::vector<hsd::i32> vec = {{1, 2, 3, 4, 5, 6, 7, 8, 9}};
    hsd::io::cout().get_stream().reserve(1024);

    for (auto [val, count] : vec.to_span().enumerate())
    {
        hsd::io::cout().print<"({}, {}) ">(val, count);
        val = val + 1;
    }

    hsd::io::cout().print<"\n">();

    for (auto& val : vec)
    {
        hsd::io::cout().print<"{} ">(val);
    }

    hsd::io::cout().print<"\n">().flush().unwrap();
}