#include "Printf.hpp"

#include <cstdio>

#include "String.hpp"
#include "Io.hpp"

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
                    int value = tbegin.getnext<int>().unwrap();
                    auto s = string::to_string(value);
                    std::fwrite(s.data(), 1, s.size(), stdout);
                    break;
                }
                case 's': {
                    auto value = tbegin.getnext<const char*>().unwrap();
                    std::fwrite(value, 1, cstring::length(value), stdout);
                    break;
                }
                case 'c': {
                    char value = tbegin.getnext<char>().unwrap();
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
