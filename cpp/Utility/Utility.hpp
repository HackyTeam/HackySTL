#pragma once

namespace hsd
{
    template< typename T1, typename T2 >
    struct pair
    {
        T1 fist;
        T2 second;
    };

    template<typename T>
    using rev_ref_t = typename std::remove_reference<T>::type;

    template<typename T>
    static constexpr rev_ref_t<T>&& move(T&& val)
    {
        return static_cast<rev_ref_t<T>&&>(val);
    }

    template <class T>
    static constexpr T&& forward(rev_ref_t<T>& val)
    {
        return static_cast<T&&>(val);
    }

    template <class T>
    static constexpr T&& forward(rev_ref_t<T>&& val)
    {
        static_assert(!std::is_lvalue_reference<T>::value,
                      "Can not forward an rvalue as an lvalue.");

        return static_cast<T&&>(val);
    }

    template<typename T1>
    static constexpr void swap(T1& first, T1& second)
    {
        auto _tmp = first;
        first = second;
        second = _tmp;
    }
}