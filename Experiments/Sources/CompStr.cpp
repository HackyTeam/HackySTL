#include <stdio.h>
#include <string_view>

template <std::string_view view>
static void func()
{
    using str_type = std::remove_reference_t<decltype(str)>;
    printf("%zu\n", sizeof(str_type));
    puts(str);
}

#define INVOKE_COMPTIME_FUNC(str)\
do\
{\
    static constexpr const char str_val[] = str;\
    func<str_val>();\
} while (0)

int main()
{
    INVOKE_COMPTIME_FUNC("test 123");
}