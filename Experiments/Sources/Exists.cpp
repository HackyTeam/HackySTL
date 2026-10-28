#include <Serialize/Io.hpp>
#include <source_location>

namespace hsd_test
{
    class source_location
    {
    private:
        const char* _file_name = "unknown";
        const char* _function_name = "unknown";
        hsd::u32 _line = 0;
        hsd::u32 _column = 0;

        using std_location_type = std::source_location;
        
        constexpr source_location(std_location_type loc)
            : _file_name{loc.file_name()}, _function_name{loc.function_name()}, _line{loc.line()}, _column{loc.column()}
        {}

    public:
        constexpr source_location() = default;

        static consteval auto current(std_location_type loc = std_location_type::current())
        {
            return source_location{loc};
        }

        constexpr const char* file_name() const
        {
            return _file_name;
        }

        constexpr const char* function_name() const
        {
            return _function_name;
        }

        constexpr hsd::u32 line() const
        {
            return _line;
        }

        constexpr hsd::u32 column() const
        {
            return _column;
        }
    };
} // namespace hsd_test

int main()
{
    [](hsd_test::source_location loc = hsd_test::source_location::current())
    {
        using namespace hsd::format_literals;
        hsd::print("{}:{}:{} at {}"_fmt, loc.file_name(), loc.line(), loc.column(), loc.function_name());
    }();
}