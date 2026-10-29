#include <Base/Result.hpp>
#include <Containers/String.hpp>
#include <Filesystem/_FSDetail.hpp>

// The headers that Path.hpp builds on must not pull in <stdio.h>. `BUFSIZ` and `stderr`
// are defined by <stdio.h>, so this file stops compiling if a stdio include comes back.
// The check runs only on POSIX. The Windows SDK and C runtime define the same macros
// without any stdio include in this library, so the check would fail on Windows.
#if !defined(HSD_PLATFORM_WINDOWS) && (defined(BUFSIZ) || defined(stderr))
#error "Result.hpp, String.hpp or _FSDetail.hpp depends on <stdio.h>"
#endif

int main()
{
    return 0;
}
