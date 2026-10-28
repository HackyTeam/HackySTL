#include <stdio.h>

class source_loc
{
private:
    const char* file_;
    const char* func_;
    int line_;

public:
    constexpr source_loc(const char* file, const char* func, int line)
        : file_{file}, func_{func}, line_{line}
    {}

    constexpr const char* file() const
    {
        return file_;
    }

    constexpr const char* func() const
    {
        return func_;
    }

    constexpr int line() const
    {
        return line_;
    }

    static consteval source_loc current(
        const char* func = __builtin_FUNCTION(),
        const char* file_name = __builtin_FILE(),
        int line = __builtin_LINE())
    {
        return source_loc{file_name, func, line};
    }
};

constexpr auto get_loc(
    source_loc loc = source_loc::current())
{
    return loc;
}

//template <typename T, unsigned Sz>
constexpr auto dummy_func(auto value, int v = 0/*const T (&)[Sz]*/)
{
    static_assert(false);
    return get_loc(source_loc::current());
}

int main()
{
    constexpr int arr[] = {1, 2, 3};
    auto loc = dummy_func(arr);
    printf("%s:%d:%s\n", loc.file(), loc.line(), loc.func());

    __builtin_COLUMN();

    return 0;
}