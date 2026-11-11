#include <Base/TypeTraits.hpp>
#include <cassert>
#include <cstddef>
#include <type_traits>

// hsd::common_type must agree with std::common_type for every list checked here. A list
// with no common type has no `type` member in either trait, and that is checked without a
// compile error, because the test instantiates the trait only through a requires-expression.

namespace
{
    struct base
    {};

    struct derived : base
    {};

    struct unrelated
    {};

    template <typename... Types>
    concept has_hsd_common = requires { typename hsd::common_type<Types...>::type; };

    template <typename... Types>
    concept has_std_common = requires { typename std::common_type<Types...>::type; };

    // Holds when both traits give the same type, or when neither gives one.
    template <typename... Types>
    constexpr bool agrees_with_std()
    {
        if constexpr (has_std_common<Types...>)
        {
            return has_hsd_common<Types...> &&
                hsd::is_same<hsd::common_type_t<Types...>, std::common_type_t<Types...>>::value;
        }
        else
        {
            return !has_hsd_common<Types...>;
        }
    }

    // Lists of one type: decayed, with references and cv-qualifiers removed.
    static_assert(agrees_with_std<int>());
    static_assert(agrees_with_std<const int>());
    static_assert(agrees_with_std<int&>());
    static_assert(agrees_with_std<const int&>());
    static_assert(agrees_with_std<int&&>());
    static_assert(agrees_with_std<int[4]>());
    static_assert(agrees_with_std<void>());
    static_assert(agrees_with_std<std::nullptr_t>());
    static_assert(hsd::is_same<hsd::common_type_t<const int&>, int>::value);
    static_assert(hsd::is_same<hsd::common_type_t<int(&)[3]>, int*>::value);

    // Pairs of arithmetic types follow the usual arithmetic conversions.
    static_assert(agrees_with_std<int, double>());
    static_assert(agrees_with_std<int, long long>());
    static_assert(agrees_with_std<unsigned, int>());
    static_assert(agrees_with_std<char, short>());
    static_assert(agrees_with_std<char, char>());
    static_assert(agrees_with_std<bool, int>());
    static_assert(agrees_with_std<float, double>());
    static_assert(hsd::is_same<hsd::common_type_t<int, double>, double>::value);

    // Pairs that differ only in references or cv-qualifiers decay to the same type first.
    static_assert(agrees_with_std<int, int&>());
    static_assert(agrees_with_std<const int, int>());
    static_assert(agrees_with_std<int&, const int&>());
    static_assert(agrees_with_std<int&, float&&>());
    static_assert(agrees_with_std<double, int&&>());
    static_assert(hsd::is_same<hsd::common_type_t<const int, int>, int>::value);

    // Arrays decay to pointers, and a null pointer constant converts to any pointer.
    static_assert(agrees_with_std<int[3], int*>());
    static_assert(agrees_with_std<int(&)[3], int*>());
    static_assert(agrees_with_std<std::nullptr_t, int*>());
    static_assert(agrees_with_std<int*, const int*>());
    static_assert(hsd::is_same<hsd::common_type_t<int[3], int*>, int*>::value);

    // Classes use the conversion between them, so a derived class gives its base.
    static_assert(agrees_with_std<derived, base>());
    static_assert(agrees_with_std<base, derived>());
    static_assert(hsd::is_same<hsd::common_type_t<derived, base>, base>::value);

    // Lists of three or more types fold from the left.
    static_assert(agrees_with_std<int, double, char>());
    static_assert(agrees_with_std<short, unsigned char, short>());
    static_assert(agrees_with_std<int, int, int, int>());
    static_assert(agrees_with_std<int*, std::nullptr_t, int[2]>());
    static_assert(agrees_with_std<derived, base, derived>());
    static_assert(hsd::is_same<hsd::common_type_t<int, double, char>, double>::value);

    // Lists with no common type, and the empty list, have no `type` member.
    static_assert(agrees_with_std<int, void>());
    static_assert(agrees_with_std<int, int*>());
    static_assert(agrees_with_std<unrelated, base>());
    static_assert(agrees_with_std<unrelated, int>());
    static_assert(agrees_with_std<int, double, void>());
    static_assert(!has_hsd_common<>);
    static_assert(!has_hsd_common<int, void>);
    static_assert(!has_hsd_common<int, double, unrelated>);

    // A function whose return type depends on the common type is simply removed from overload
    // resolution when there is none, rather than failing to compile.
    template <typename A, typename B>
    auto combine(A a, B b) -> hsd::common_type_t<A, B>
    {
        return a + b;
    }

    int combine(...)
    {
        return -1;
    }

    static_assert(hsd::is_same<decltype(combine(1, 2.0)), double>::value);
}

int main()
{
    // The common type of a mixed pair is the type the values are combined in.
    using mixed = hsd::common_type_t<int, double>;
    assert(static_cast<mixed>(7) / 2 == 3.5);

    // A decayed array and pointer share one type, so the array's first element is reachable.
    int values[3] = {4, 5, 6};
    hsd::common_type_t<int[3], int*> first = values;
    assert(*first == 4);

    // A list without a common type reports -1 through the fallback overload.
    assert(combine(1, nullptr) == -1);
    return 0;
}
