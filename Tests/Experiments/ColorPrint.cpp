#include <stdio.h>
#include <stdint.h>

#define BIT8_COLOR_RESET   L"\x1b[0m"
#define BIT8_COLOR_FMT     L"\x1b[38;5;%hhum"

template <typename CharT>
struct format_info
{
    const CharT* format;
    size_t length;

    enum print_tag : unsigned char
    {
        none = 0, 
        hex  = 1, // hexadecimal
        exp  = 2, // exponent
        fg   = 4, // foreground color
        bg   = 8, // background color
    } tag;

    unsigned char foreground;
    unsigned char background;
};

int main()
{
    unsigned char i = 0;

    while (i != 255)
    {
        wprintf(
            "TEST BEFORE "
            BIT8_COLOR_FMT 
            L"\x1b[48;5;%hhum" 
            L"Test Text" BIT8_COLOR_RESET "\n", 
            i, static_cast<unsigned char>(255) - i
        );

        i++;
    }

    wprintf(
            "TEST BEFORE "
            BIT8_COLOR_FMT 
            L"\x1b[48;5;%hhum" 
            L"Test Text" BIT8_COLOR_RESET "\n", 
            static_cast<unsigned char>(255), static_cast<unsigned char>(0)
        );
}