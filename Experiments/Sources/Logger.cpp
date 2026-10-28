#include <Threading/Time.hpp>
#include <Filesystem/Path.hpp>
#include <Serialize/Io.hpp>

namespace dgb
{
    static inline void msg(
        const char* message,
        hsd::source_location loc = hsd::source_location::current()
    )
    {
        using namespace hsd::format_literals;
        hsd::filesystem::path path = loc.file_name();

        hsd::date current_date;
        hsd::println(
            "[{fg=21}][{fg=220}:{fg=220}:{fg=220}][{}:{}:{}] {}"_fmt, 
            "INFO",
            current_date.get_time().get_hour(),
            current_date.get_time().get_minutes(),
            current_date.get_time().get_seconds(),
            path.relative_name(), loc.function_name(), loc.line(), message
        );
        // [info][hh:mm:ss][file:function:line] message
    }
}

namespace longname::name::template_name
{
    template <typename... Ts>
    static inline void func(Ts...)
    {
        dgb::msg("Test msg");
    }
}

int main()
{
    longname::name::template_name::func(0, true, 'c', "aaa", 1.33);
}