#include <Io.hpp>

template <hsd::basic_string_literal fmt>
consteval auto operator""_fmt()
{
    return [] {
        return fmt;
    };
}

template <typename T, typename... Args>
static void println(T&&, Args&&... args)
{
    static constexpr T _func{};
    hsd::io::cout().template print<_func() + "\n">(hsd::forward<Args>(args)...).flush().unwrap();
}

int main()
{
    hsd::io::cout().get_stream().reserve(1024);
    print("Hello, world!"_fmt);
}