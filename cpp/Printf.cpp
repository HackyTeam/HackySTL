#include "Printf.hpp"

#include <cstdio>
#include <cstring>

int xvprintf(const char* fmt, xtuple_iterator tbegin, xtuple_iterator tend) {
    while (*fmt) {
        char ch = *fmt++;
        if (ch != '%')
            std::putchar(ch);
        else {
            char fch = *fmt++;
            if (!fch) {
                std::fputs("\nError: Invalid format string", stderr);
                return 123;
            }
            if (tbegin == tend) {
                std::fputs("\nError: Passed to few arguments", stderr);
                return 1234;
            }
            switch (fch) {
                case 'd': {
                    int value = tbegin.getnext<int>();
                    std::printf("%d", value);
                    break;
                }
                case 's': {
                    auto value = tbegin.getnext<char const*>();
                    std::fwrite(value, 1, std::strlen(value), stdout);
                    break;
                }
                case 'c': {
                    char value = tbegin.getnext<char>();
                    std::putchar(value);
                    break;
                }
                default: {
                    std::fputs("\nError: Invalid format specifier", stderr);
                    return 12345;
                }
            }
        }
    }
    if (tbegin != tend) {
        std::fputs("\nError: Not all arguments parsed", stderr);
        return -666;
    }
    return 0;
}
