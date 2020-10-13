#include "../../cpp/Limits.hpp"
#include "../../cpp/Io.hpp"

int main()
{
    hsd::io::print<"the limit is: {}\n">(1.e+10f + 1.e-7f);
}