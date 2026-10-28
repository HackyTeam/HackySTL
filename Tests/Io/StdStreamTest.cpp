#include <Serialize/Io.hpp>
#include <cassert>

#if defined(HSD_PLATFORM_POSIX)
#include <fcntl.h>
#endif

namespace
{
    /// @brief Checks that the standard descriptors are still open at exit.
    /// @details Constructed before the first use of `io::cout`, so it is
    /// destroyed after the `io` static that wraps standard output.
    struct std_descriptor_checker
    {
        ~std_descriptor_checker()
        {
            #if defined(HSD_PLATFORM_POSIX)
            assert(fcntl(1, F_GETFD) != -1);
            assert(fcntl(2, F_GETFD) != -1);
            #endif
        }
    };
}

int main()
{
    using namespace hsd::format_literals;

    static std_descriptor_checker _checker;
    hsd::println("standard output is still open after io output"_fmt);

    return 0;
}
