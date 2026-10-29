#include <Base/Result.hpp>
#include <Containers/String.hpp>
#include <Filesystem/_FSDetail.hpp>

#if defined(HSD_PLATFORM_WINDOWS)
// On POSIX, Path.hpp still includes <stdio.h> for rename(), so only Windows checks it.
#include <Filesystem/Path.hpp>
#endif

// The headers that Path.hpp builds on must not pull in <stdio.h>. `BUFSIZ` and `stderr`
// are defined by <stdio.h>, so this file stops compiling if a stdio include comes back.
#if defined(BUFSIZ) || defined(stderr)
#error "Result.hpp, String.hpp or Path.hpp depends on <stdio.h>"
#endif

int main()
{
    return 0;
}
