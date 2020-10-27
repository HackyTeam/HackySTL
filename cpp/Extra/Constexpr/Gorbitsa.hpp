#pragma once

#include "../../StackArray.hpp"
#include "../../TypeTraits.hpp"

// Credits:
// -DeKrain
// -qookie
//
namespace hsd
{
    template < const char* s, size_t i, char f, char... ss >
    struct _str_to_arr
        : _str_to_arr< s, i + 1, s[i + 1], ss..., f > 
    {};

    template < const char* s, size_t i, char... ss >
    struct _str_to_arr<s, i, 0, ss...> 
    {
        using type = integer_sequence<char, ss...>;
    };

    template <const char* s>
    struct str_to_arr
        : _str_to_arr<s, 0, s[0]>
    {};

    template <typename arr>
    struct arr_size;

    template < typename T, T... v >
    struct arr_size< integer_sequence<T, v...> >
        : literal_constant<size_t, sizeof...(v)> 
    {};

    template <typename T>
    static constexpr inline auto arr_size_v = arr_size<T>::value;

    template <typename arr, size_t idx>
    struct arr_elem;

    template < typename T, T a, T... b >
    struct arr_elem< integer_sequence<T, a, b...>, 0 >
        : literal_constant<decltype(a), a> 
    {};

    template < typename T, usize idx, T a, T... b >
    struct arr_elem< integer_sequence<T, a, b...>, idx >
        : arr_elem< integer_sequence<T, b...>, idx - 1 > 
    {};

    template <typename T, usize idx>
    static constexpr inline auto arr_elem_v = arr_elem<T, idx>::value;

    template < typename T, T v, usize count, T... vs >
    struct arr_makefill 
    {
        using type = typename arr_makefill<T, v, count - 1, vs..., v>::type;
    };

    template < typename T, T v, T... vs >
    struct arr_makefill< T, v, 0, vs... > 
    {
        using type = integer_sequence<T, vs...>;
    };

    template <typename arr, auto v>
    struct arr_append;

    template < typename T, T... vs, T v >
    struct arr_append<integer_sequence<T, vs...>, v> 
    {
        using type = integer_sequence<T, vs..., v>;
    };

    template < typename arr, auto v >
    using arr_append_t = typename arr_append<arr, v>::type;

    template < typename arr, typename narr, usize idx, auto nv >
    struct _arr_replace;

    template < typename T, T... nvs, T va, T... vs, T nv >
    struct _arr_replace< integer_sequence<T, va, vs...>, integer_sequence<T, nvs...>, 0, nv >
    {
        using type = integer_sequence< T, nvs..., nv, vs... >;
    };

    template < typename T, usize idx, T... nvs, T va, T... vs, T nv >
    struct _arr_replace< integer_sequence<T, va, vs...>, integer_sequence<T, nvs...>, idx, nv >
        : _arr_replace< integer_sequence<T, vs...>, integer_sequence<T, nvs..., va>, idx - 1, nv > 
    {};

    template < typename arr, usize idx, auto nv >
    struct arr_replace;

    template < typename T, T... vs, usize idx, auto nv >
    struct arr_replace< integer_sequence<T, vs...>, idx, nv > 
        : _arr_replace< integer_sequence<T, vs...>, integer_sequence<T>, idx, nv > 
    {};

    template < typename arr, size_t idx, auto nv >
    using arr_replace_t = typename arr_replace<arr, idx, nv>::type;

    template <typename arr>
    struct arr_collapse;

    template < typename T, T... vs >
    struct arr_collapse< integer_sequence<T, vs...> > 
    {
        static constexpr inline auto value = stack_array<T, sizeof...(vs)>{vs...};
    };


    template < usize PC, u8 X, typename Ram, typename Out, typename Prog >
    consteval auto run_impl() 
    {
        if constexpr (PC + 1 >= arr_size_v<Prog>)
        { 
            return arr_collapse<Out>::value;
        }
        else 
        {
            constexpr uint8_t op = arr_elem_v<Prog, PC>;
            constexpr uint8_t arg = arr_elem_v<Prog, PC+1>;

            if constexpr (op == 'G')
                return run_impl<PC + 2, arr_elem_v<Ram, arg>, Ram, Out, Prog>();
            if constexpr (op == 'O')
                return run_impl<PC + 2, X, arr_replace_t<Ram, arg, X>, Out, Prog>();
            if constexpr (op == 'R')
                return run_impl<PC + 2, /*input*/ 0, Ram, Out, Prog>();
            if constexpr (op == 'B')
                return run_impl<X ? PC + 2 : arg * 2, X, Ram, Out, Prog>();
            if constexpr (op == 'I')
                return run_impl<PC + 2, (X + arg) & 0xFF, Ram, Out, Prog>();
            if constexpr (op == 'T') // output
                return run_impl<PC + 2, X, Ram, arr_append_t<Out, X>, Prog>();
            if constexpr (op == 'S')
                return run_impl<PC + 2, arg, Ram, Out, Prog>();
            if constexpr (op == 'A')
                return run_impl<PC + 2, (X + arr_elem_v<Ram, arg>) & 0xFF, Ram, Out, Prog>();
            if constexpr (op == 'g')
                return run_impl<PC + 2, arr_elem_v<Ram, arr_elem_v<Ram, arg>>, Ram, Out, Prog>();
            if constexpr (op == 'o')
                return run_impl<PC + 2, X, arr_replace_t<Ram, arr_elem_v<Ram, arg>, X>, Out, Prog>();
            if constexpr (op == 'r')
                return run_impl<PC + 2, X, arr_replace_t<Ram, arg, /*input*/ 0>, Out, Prog>();
            if constexpr (op == 'b')
                return run_impl<X ? PC + 2 : arr_elem_v<Ram, arg> * 2, X, Ram, Out, Prog>();
            if constexpr (op == 'i')
                return run_impl<PC + 2, X, arr_replace_t<Ram, arg, (arr_elem_v<Ram, arg> + X) & 0xFF>, Out, Prog>();
            if constexpr (op == 't')
                return run_impl<PC + 2, X, Ram, arr_append_t<Out, arr_elem_v<Ram, arg>>, Prog>();
            if constexpr (op == 's')
                return run_impl<PC + 2, X ^ arr_elem_v<Ram, arg>, Ram, Out, Prog>();
            if constexpr (op == 'a')
                return run_impl<PC + 2, (X + arr_elem_v<Ram, arr_elem_v<Ram, arg>>) & 0xFF, Ram, Out, Prog>();
            //if constexpr (op == ' ')
            //    return run_impl<PC + 2, X, Ram, Out, Prog>();
        }
    }

    template < typename Str, bool Parsing_command, u8 Tmp, usize I, u8... c >
    consteval auto run_parser() 
    {
    	if constexpr (arr_size_v<Str> > I) 
        {
    		if constexpr (arr_elem_v<Str, I> == 'R' || arr_elem_v<Str, I> == 'T') 
            {
    			if constexpr (arr_size_v<Str> > I + 1)
    				static_assert(arr_elem_v<Str, I + 1> == ' ');
    			return run_parser<Str, true, 0, I + 2, c..., arr_elem_v<Str, I>, 0>();
    		} 
            else 
            {
    			if constexpr (Parsing_command)
    				return run_parser<Str, false, 0, I + 1, c..., arr_elem_v<Str, I>>();
    			else if constexpr (arr_elem_v<Str, I> >= '0' && arr_elem_v<Str, I> <= '9')
    				return run_parser<Str, false, (Tmp * 10 + arr_elem_v<Str, I> - '0') & 0xFF, I + 1, c...>();
    			else 
                {
    				static_assert(arr_elem_v<Str, I> == ' ');
    				return run_parser<Str, true, 0, I + 1, c..., Tmp>();
    			}
    		}
    	} 
        else 
        {
    		return run_impl<
                0, 0, typename arr_makefill<uint8_t, 0, 256>::type, 
                integer_sequence<u8>, integer_sequence<u8, c...> >();
    	}
    }
}