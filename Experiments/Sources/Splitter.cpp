#include "../Headers/Splitter.hpp"

int main()
{
    constexpr auto val = hsd::sstream_detail::parse_literal<
        "{hex,foreground=234,background=43}, \
        {exp,foreground=143,background=78}, \
        {foreground=84,background=193}", 4
    >().unwrap();

    return 0;
}