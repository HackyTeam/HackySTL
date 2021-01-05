#include "Printf.hpp"

#include <cstdio>
#include <cstring>

using namespace hsd;
Result<void, runtime_error> hsd::xvprintf(const char* fmt, xtuple_iterator tbegin, xtuple_iterator tend) noexcept {
    while (*fmt) {
        char ch = *fmt++;
        if (ch != '%')
            std::putchar(ch);
        else {
            char fch = *fmt++;
            if (!fch) {
                return runtime_error("\nError: Invalid format string");
            }
            if (tbegin == tend) {
                return runtime_error("\nError: Passed to few arguments");
            }
            switch (fch) {
                case 'd': {
                    int value = tbegin.getnext<int>();
                    std::printf("%d", value);
                    break;
                }
                case 's': {
                    auto value = tbegin.getnext<const char*>();
                    std::fwrite(value, 1, std::strlen(value), stdout);
                    break;
                }
                case 'c': {
                    char value = tbegin.getnext<char>();
                    std::putchar(value);
                    break;
                }
                default: {
                    return runtime_error("\nError: Invalid format specifier");
                }
            }
        }
    }
    if (tbegin != tend) {
        return runtime_error("\nError: Not all arguments parsed");
    }
    return {};
}
