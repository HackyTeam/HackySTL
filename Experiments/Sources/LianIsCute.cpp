#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <hsd::usize N>
struct lian_is_cute 
{
    template <typename CharT>
    static consteval auto pretty_fmt()
    {
        static_assert(
            hsd::is_same<CharT, char>::value ||
            hsd::is_same<CharT, hsd::wchar>::value,
            "CharT must be either char or hsd::wchar"
        );

        if constexpr (hsd::is_same<CharT, char>::value)
        {
            constexpr auto _fmt = 
                "{italic,fg=9}, {ovr}"_fmt;

            return _fmt();
        }
        else
        {
            constexpr auto _fmt = 
                L"{italic,fg=9}, {ovr}"_fmt;

            return _fmt();
        }
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<const char*, lian_is_cute<N - 1>>
    {
        return {"Lian is cute", lian_is_cute<N - 1>{}};
    }
};

template <>
struct lian_is_cute<0> 
{
    template <typename CharT>
    static consteval auto pretty_fmt()
    {
        static_assert(
            hsd::is_same<CharT, char>::value ||
            hsd::is_same<CharT, hsd::wchar>::value,
            "CharT must be either char or hsd::wchar"
        );

        if constexpr (hsd::is_same<CharT, char>::value)
        {
            constexpr auto _fmt = 
                "{italic,fg=9}"_fmt;

            return _fmt();
        }
        else
        {
            constexpr auto _fmt = 
                L"{italic,fg=9}"_fmt;

            return _fmt();
        }
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<const char*>
    {
        return {"Lian is cute"};
    }
};

int main()
{
    hsd::println("{ovr}"_fmt, lian_is_cute<10>{});
}