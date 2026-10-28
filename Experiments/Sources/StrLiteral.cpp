#include <type_traits>

template <typename InIt, typename OutIt>
static constexpr OutIt copy_n(InIt first, unsigned n, OutIt dest)
{
    using value_type = std::remove_reference_t<decltype(*dest)>;

    for (unsigned _index = 0; _index != n; _index++)
    {
        *dest++ = static_cast<value_type>(*first++);
    }

    return dest;
}

template <unsigned N>
struct string_literal
{
    char data[N]{};
    using char_type = char;

    string_literal() = default;

    string_literal(const char (&str)[N])
    {
        copy_n(str, N, data);
    }

    string_literal(const char* str, unsigned len)
    {
        copy_n(str, len, data);
    }

    template <unsigned N2>
    auto operator+(
        const string_literal<N2>& rhs) const
    {
        string_literal<N + N2 - 1> _str;
        copy_n(data, N, _str.data);
        copy_n(rhs.data, N2, (_str.data + N - 1));
        return _str;
    }

    template <unsigned N2>
    auto operator+(const char (&rhs)[N2]) const
    {
        string_literal<N + N2 - 1> _str;
        copy_n(data, N, _str.data);
        copy_n(rhs, N2, (_str.data + N - 1));
        return _str;
    }

    template <unsigned N2>
    friend auto operator+(
        const char (&lhs)[N2], 
        const string_literal& rhs)
    {
        string_literal<N + N2 - 1> _str;
        copy_n(lhs, N2, _str.data);
        copy_n(rhs.data, N, (_str.data + N2 - 1));
        return _str;
    }

    unsigned size() const
    {
        return N;
    }
};

int main()
{
    auto res = 
    "\x1b[38;5;%hhum" + 
    string_literal<8>{"hello, "} + "%lf" + "\x1b[0m";
    
}