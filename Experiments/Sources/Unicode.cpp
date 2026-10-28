#include <assert.h>
#include <CString.hpp>

int main()
{
    // UTF-8 to UTF-32
    {
        wchar_t expected = L'🌠';
        const char8_t* test = u8"🌠";
        wchar_t result = 0;

        // 110 xxxxx 10 xxxxxx -> 11 bytes
        // 1110 xxxx 10 xxxxxx 10 xxxxxx -> 16 bytes
        // 11110 xxx 10 xxxxxx 10 xxxxxx 10 xxxxxx -> 21 bytes

        if ((test[0] & 0xF8) == 0xF0 && (test[1] & 0xC0) == 0x80 && (test[2] & 0xC0) == 0x80 && (test[3] & 0xC0) == 0x80)
        {
            result = (test[0] & 0x07) << 18 | (test[1] & 0x3F) << 12 | (test[2] & 0x3F) << 6 | (test[3] & 0x3F);
            assert(result == expected);
        }
        else if ((test[0] & 0xF0) == 0xE0 && (test[1] & 0xC0) == 0x80 && (test[2] & 0xC0) == 0x80)
        {
            result = (test[0] & 0x0F) << 12 | (test[1] & 0x3F) << 6 | (test[2] & 0x3F);
            assert(result == expected);
        }
        else if ((test[0] & 0xE0) == 0xC0 && (test[1] & 0xC0) == 0x80)
        {
            result = (test[0] & 0x1F) << 6 | (test[1] & 0x3F);
            assert(result == expected);
        }
        else
        {
            result = test[0];
            assert(result == expected);
        }
    }

    // UTF-32 to UTF-8
    {
        const char8_t* expected = u8"🌠";
        const wchar_t test = L'🌠';
        char8_t result[5]{};

        // 4 bytes
        if (test < 0x7FFFF)
        {
            result[0] = 0xFF & (0xF0 | (test >> 18));
            result[1] = 0xFF & (0x80 | ((test >> 12) & 0x3F));
            result[2] = 0xFF & (0x80 | ((test >> 6) & 0x3F));
            result[3] = 0xFF & (0x80 | (test & 0x3F));
            result[4] = 0;
        }
        // 3 bytes
        else if (test < 0x1FFF)
        {
            result[0] = 0xFF & (0xE0 | (test >> 12));
            result[1] = 0xFF & (0x80 | ((test >> 6) & 0x3F));
            result[2] = 0xFF & (0x80 | (test & 0x3F));
            result[3] = 0;
        }
        // 2 bytes
        else if (test < 0x7FF)
        {
            result[0] = 0xFF & (0xC0 | (test >> 6));
            result[1] = 0xFF & (0x80 | (test & 0x3F));
            result[2] = 0;
        }
        // 1 byte
        else
        {
            result[0] = test & 0xFF;
            result[1] = 0;
        }

        assert(hsd::u8cstring::compare(result, expected) == 0);
    }

    // UTF-16 to UTF-32
    
    {
        const wchar_t expected = L'𒈙';
        const char16_t* test = u"𒈙";
        wchar_t result = 0;

        if ((test[0] & 0xFC00) == 0xD800 && (test[1] & 0xFC00) == 0xDC00)
        {
            result = (((test[0] & 0x3FF) << 10) | (test[1] & 0x3FF)) + 0x10000;
            assert(result == expected);
        }
        else
        {
            result = test[0];
            assert(result == expected);
        }
    }

    // UTF-32 to UTF-16
    {
        const char16_t* expected = u"𒈙";
        const wchar_t test = L'𒈙';
        char16_t result[3]{};

        if (test < 0x10000)
        {
            result[0] = test & 0xFFFF;
            result[1] = 0;
        }
        else
        {
            result[0] = 0xD800 | ((test - 0x10000) >> 10);
            result[1] = 0xDC00 | ((test - 0x10000) & 0x3FF);
            result[2] = 0;
        }

        assert(hsd::u16cstring::compare(result, expected) == 0);
    }

    // UTF-8 to UTF-16
    {
        const char16_t* expected = u"🌠";
        const char8_t* test = u8"🌠";
        char16_t result[3]{};

        // 4 bytes
        if ((test[0] & 0xF8) == 0xF0 && (test[1] & 0xC0) == 0x80 && (test[2] & 0xC0) == 0x80 && (test[3] & 0xC0) == 0x80)
        {
            int expr = ((test[0] & 0x07) << 18) | ((test[1] & 0x3F) << 12) | ((test[2] & 0x3F) << 6) | (test[3] & 0x3F);
            result[0] = 0xD800 | ((expr - 0x10000) >> 10);
            result[1] = 0xDC00 | ((expr - 0x10000) & 0x3FF);
            result[2] = 0;
        }
        // 3 bytes
        else if ((test[0] & 0xF0) == 0xE0 && (test[1] & 0xC0) == 0x80 && (test[2] & 0xC0) == 0x80)
        {
            int expr = ((test[0] & 0x0F) << 12) | ((test[1] & 0x3F) << 6) | (test[2] & 0x3F);
            
            if (expr < 0x10000)
            {
                result[0] = expr & 0xFFFF;
                result[1] = 0;
            }
            else
            {
                result[0] = 0xD800 | ((expr - 0x10000) >> 10);
                result[1] = 0xDC00 | ((expr - 0x10000) & 0x3FF);
                result[2] = 0;
            }
        }
        // 2 bytes
        else if ((test[0] & 0xE0) == 0xC0 && (test[1] & 0xC0) == 0x80)
        {
            result[0] = ((test[0] & 0x1F) << 6) | (test[1] & 0x3F);
            result[1] = 0;
        }
        // 1 byte
        else
        {
            result[0] = test[0];
            result[1] = 0;
        }

        assert(hsd::u16cstring::compare(result, expected) == 0);
    }

    // UTF-16 to UTF-8
    {
        const char8_t* expected = u8"🌠";
        const char16_t* test = u"🌠";
        char8_t result[5]{};
        int expr = 0;

        // 4 bytes
        if ((test[0] & 0xFC00) == 0xD800 && (test[1] & 0xFC00) == 0xDC00)
        {
            expr = (((test[0] & 0x3FF) << 10) | (test[1] & 0x3FF)) + 0x10000;
        }
        else
        {
            expr = test[0];
        }

        if (expr < 0x10000)
        {
            if (expr < 0x80)
            {
                result[0] = expr & 0xFF;
                result[1] = 0;
            }
            else if (expr < 0x800)
            {
                result[0] = 0xC0 | ((expr >> 6) & 0x1F);
                result[1] = 0x80 | (expr & 0x3F);
                result[2] = 0;
            }
            else if (expr < 0x10000)
            {
                result[0] = 0xE0 | ((expr >> 12) & 0x0F);
                result[1] = 0x80 | ((expr >> 6) & 0x3F);
                result[2] = 0x80 | (expr & 0x3F);
                result[3] = 0;
            }
        }
        else
        {
            result[0] = 0xF0 | (expr >> 18);
            result[1] = 0x80 | ((expr >> 12) & 0x3F);
            result[2] = 0x80 | ((expr >> 6) & 0x3F);
            result[3] = 0x80 | (expr & 0x3F);
            result[4] = 0;
        }
        
        assert(hsd::u8cstring::compare(result, expected) == 0);
    }
}