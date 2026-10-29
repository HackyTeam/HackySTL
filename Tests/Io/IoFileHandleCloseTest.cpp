#include <cassert>

#include <Serialize/_IoDetail.hpp>

#if !defined(HSD_PLATFORM_WINDOWS)
#include <fcntl.h>
#include <unistd.h>
#endif

// Regression test: a file_handler opened with `open` must close its descriptor when it
// is destroyed. POSIX hands out the lowest free descriptor number, so after the handler
// is gone the next open returns the same number. Windows handles are closed by the
// destructor too, but this check is written for POSIX descriptors.
int main()
{
#if !defined(HSD_PLATFORM_WINDOWS)
    const int _first = ::open(__FILE__, O_RDONLY);
    assert(_first != -1);
    ::close(_first);

    {
        auto _handle = hsd::io_detail::file_handler::open(__FILE__, hsd::io_options::read);
        assert(_handle.is_open());
    }

    const int _second = ::open(__FILE__, O_RDONLY);
    assert(_second == _first);
    ::close(_second);
#endif

    return 0;
}
