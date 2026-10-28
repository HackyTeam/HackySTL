#include "../Headers/RyuGeneric.hpp"
#include <Serialize/Io.hpp>

int main()
{
    using namespace hsd::format_literals;
    hsd::f64 val = 0.43534254234435234523453425435L;
    auto str = hsd::ryu_generic::to_string(val, hsd::ryu_generic::Format::Fixed);

    hsd::println("value is: {fg=255} -> {}"_fmt, val, str);
}