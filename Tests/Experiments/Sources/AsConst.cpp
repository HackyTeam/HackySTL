#include <stdio.h>

template <typename T>
static constexpr void as_const(T&&) = delete;

template <typename T>
static constexpr const T& as_const(T& val)
{
    return val;
}

template <typename T>
static constexpr const T* as_const(T* val)
{
    return val;
}

template <typename T>
static constexpr const T& as_const(const T& val)
{
    return val;
}

template <typename T>
static constexpr const T* as_const(const T* val)
{
    return val;
}

int main()
{
    char c[16]{};

    as_const(c);
}