#include <Network/Network.hpp>

// The network headers report errors through the same writer as Result.hpp, so they must not
// pull in <stdio.h>. `BUFSIZ` and `stderr` are defined by <stdio.h>, so this file stops
// compiling if a stdio include comes back. The check runs only on POSIX, because the Windows
// SDK and C runtime define the same macros.
#if !defined(HSD_PLATFORM_WINDOWS) && (defined(BUFSIZ) || defined(stderr))
#error "The network headers depend on <stdio.h>"
#endif

int main()
{
    return 0;
}
