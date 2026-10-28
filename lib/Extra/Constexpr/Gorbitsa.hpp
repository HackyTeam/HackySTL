#pragma once

/// @file Gorbitsa.hpp
/// @brief Compile-time parser and renderer for the Gorbitsa language.
/// @details Interprets compact drawing commands at compile time and returns a
/// fixed sequence suitable for constexpr tests and generated output.

#include "../../Containers/StackArray.hpp"
#include "../../Base/IntegerSequence.hpp"

///
/// Credits:
/// -DeKrain
/// -qookie
///

namespace hsd
{
    /// @brief Recursive helper collecting the characters of a null-terminated
    /// string literal into a parameter pack, one per template instantiation.
    /// @tparam s Pointer to the source string literal.
    /// @tparam i Index of the character currently being consumed.
    /// @tparam f Character at index `i`, inspected to detect the terminator.
    /// @tparam ss Characters collected so far, in original order.
    template < const char* s, usize i, char f, char... ss >
    struct _str_to_arr
        : _str_to_arr< s, i + 1, s[i + 1], ss..., f > 
    {};

    /// @brief Terminates collection once the null terminator is reached.
    /// @tparam s Pointer to the source string literal.
    /// @tparam i Index of the terminating null character.
    /// @tparam ss All characters collected before the terminator.
    template < const char* s, usize i, char... ss >
    struct _str_to_arr<s, i, 0, ss...> 
    {
        /// @brief Collected characters, excluding the null terminator.
        using type = integer_sequence<char, ss...>;
    };

    /// @brief Converts a null-terminated string literal into a compile-time
    /// character sequence.
    /// @tparam s Pointer to the source string literal.
    template <const char* s>
    struct str_to_arr
        : _str_to_arr<s, 0, s[0]>
    {};

    /// @brief Compile-time element count of an integer_sequence-like array.
    /// @tparam arr Sequence type whose size is queried.
    template <typename arr>
    struct arr_size;

    /// @brief Element count of an integer_sequence, exposed as `value`.
    /// @tparam T Element type of the sequence.
    /// @tparam v Elements of the sequence.
    template < typename T, T... v >
    struct arr_size< integer_sequence<T, v...> >
        : literal_constant<usize, sizeof...(v)> 
    {};

    /// @brief Convenience value alias for arr_size<T>::value.
    /// @tparam T Sequence type whose size is queried.
    template <typename T>
    static constexpr auto arr_size_v = arr_size<T>::value;

    /// @brief Compile-time indexed element access into an integer_sequence.
    /// @tparam arr Sequence type being indexed.
    /// @tparam idx Zero-based index of the requested element.
    template <typename arr, usize idx>
    struct arr_elem;

    /// @brief Base case: exposes the head element as `value`.
    /// @tparam T Element type of the sequence.
    /// @tparam a Head element, returned as `value`.
    /// @tparam b Remaining elements, ignored.
    template < typename T, T a, T... b >
    struct arr_elem< integer_sequence<T, a, b...>, 0 >
        : literal_constant<decltype(a), a> 
    {};

    /// @brief Recursive case: drops the head element and decrements `idx`.
    /// @tparam T Element type of the sequence.
    /// @tparam idx Remaining index to walk down to 0.
    /// @tparam a Head element, discarded.
    /// @tparam b Remaining elements, searched recursively.
    template < typename T, usize idx, T a, T... b >
    struct arr_elem< integer_sequence<T, a, b...>, idx >
        : arr_elem< integer_sequence<T, b...>, idx - 1 > 
    {};

    /// @brief Convenience value alias for arr_elem<T, idx>::value.
    /// @tparam T Sequence type being indexed.
    /// @tparam idx Zero-based index of the requested element.
    template <typename T, usize idx>
    static constexpr auto arr_elem_v = arr_elem<T, idx>::value;

    /// @brief Recursively builds a sequence of `count` copies of `v`.
    /// @tparam T Element type of the produced sequence.
    /// @tparam v Value repeated in every produced element.
    /// @tparam count Remaining number of copies to append.
    /// @tparam vs Copies of `v` accumulated so far.
    template < typename T, T v, usize count, T... vs >
    struct arr_makefill 
    {
        /// @brief Sequence containing `count` additional copies of `v`.
        using type = typename arr_makefill<T, v, count - 1, vs..., v>::type;
    };

    /// @brief Terminates once no further copies remain to be appended.
    /// @tparam T Element type of the produced sequence.
    /// @tparam v Value repeated in the sequence (unused at this depth).
    /// @tparam vs All copies accumulated so far.
    template < typename T, T v, T... vs >
    struct arr_makefill< T, v, 0, vs... > 
    {
        /// @brief Sequence containing the accumulated copies of `v`.
        using type = integer_sequence<T, vs...>;
    };

    /// @brief Appends a single value to the end of an integer_sequence.
    /// @tparam arr Sequence type to append to.
    /// @tparam v Value appended after the sequence's existing elements.
    template <typename arr, auto v>
    struct arr_append;

    /// @brief Produces a sequence with `v` appended after `vs...`.
    /// @tparam T Element type of the sequence.
    /// @tparam vs Existing elements, kept in order.
    /// @tparam v Value appended at the end.
    template < typename T, T... vs, T v >
    struct arr_append<integer_sequence<T, vs...>, v> 
    {
        /// @brief Sequence with `v` appended to the original elements.
        using type = integer_sequence<T, vs..., v>;
    };

    /// @brief Convenience alias for arr_append<arr, v>::type.
    /// @tparam arr Sequence type to append to.
    /// @tparam v Value appended after the sequence's existing elements.
    template < typename arr, auto v >
    using arr_append_t = typename arr_append<arr, v>::type;

    /// @brief Recursive helper that walks toward index `idx`, moving
    /// already-visited elements from `arr` into the accumulator `narr`.
    /// @tparam arr Remaining, not-yet-visited tail of the original sequence.
    /// @tparam narr Elements already visited, kept in original order.
    /// @tparam idx Remaining number of elements to skip before replacing.
    /// @tparam nv Replacement value to substitute once `idx` reaches 0.
    template < typename arr, typename narr, usize idx, auto nv >
    struct _arr_replace;

    /// @brief Base case: substitutes the head of `arr` for `nv` and
    /// reattaches the untouched tail after it.
    /// @tparam T Element type of the sequence.
    /// @tparam nvs Elements visited before the replacement index.
    /// @tparam va Original element at the replacement index, discarded.
    /// @tparam vs Elements after the replacement index, kept unchanged.
    /// @tparam nv Replacement value written in place of `va`.
    template < typename T, T... nvs, T va, T... vs, T nv >
    struct _arr_replace< integer_sequence<T, va, vs...>, integer_sequence<T, nvs...>, 0, nv >
    {
        /// @brief Sequence identical to the input except for the
        /// replaced element at the target index.
        using type = integer_sequence< T, nvs..., nv, vs... >;
    };

    /// @brief Recursive case: moves the current head element into the
    /// accumulator and decrements the remaining index.
    /// @tparam T Element type of the sequence.
    /// @tparam idx Remaining number of elements to skip.
    /// @tparam nvs Elements visited so far.
    /// @tparam va Head element of the remaining tail, moved into `nvs`.
    /// @tparam vs Elements after `va`, still to be visited.
    /// @tparam nv Replacement value to substitute once `idx` reaches 0.
    template < typename T, usize idx, T... nvs, T va, T... vs, T nv >
    struct _arr_replace< integer_sequence<T, va, vs...>, integer_sequence<T, nvs...>, idx, nv >
        : _arr_replace< integer_sequence<T, vs...>, integer_sequence<T, nvs..., va>, idx - 1, nv > 
    {};

    /// @brief Produces a sequence with the element at `idx` replaced by `nv`.
    /// @tparam arr Original sequence.
    /// @tparam idx Zero-based index of the element to replace.
    /// @tparam nv Replacement value.
    template < typename arr, usize idx, auto nv >
    struct arr_replace;

    /// @brief Entry point that kicks off `_arr_replace` with an empty
    /// accumulator.
    /// @tparam T Element type of the sequence.
    /// @tparam vs Elements of the original sequence.
    /// @tparam idx Zero-based index of the element to replace.
    /// @tparam nv Replacement value.
    template < typename T, T... vs, usize idx, auto nv >
    struct arr_replace< integer_sequence<T, vs...>, idx, nv > 
        : _arr_replace< integer_sequence<T, vs...>, integer_sequence<T>, idx, static_cast<T>(nv) > 
    {};

    /// @brief Convenience alias for arr_replace<arr, idx, nv>::type.
    /// @tparam arr Original sequence.
    /// @tparam idx Zero-based index of the element to replace.
    /// @tparam nv Replacement value.
    template < typename arr, usize idx, auto nv >
    using arr_replace_t = typename arr_replace<arr, idx, nv>::type;

    /// @brief Converts a compile-time integer_sequence into a runtime-usable
    /// stack_array constant.
    /// @tparam arr Sequence type to collapse.
    template <typename arr>
    struct arr_collapse;

    /// @brief Materializes the sequence's elements as `value`.
    /// @tparam T Element type of the sequence.
    /// @tparam vs Elements copied into the resulting array, in order.
    template < typename T, T... vs >
    struct arr_collapse< integer_sequence<T, vs...> > 
    {
        /// @brief stack_array holding a copy of `vs...`, sized to match.
        static constexpr auto value = stack_array<T, sizeof...(vs)>{vs...};
    };


    /// @brief Compile-time bytecode interpreter for the Gorbitsa virtual
    /// machine.
    /// @tparam PC Program counter: index of the current opcode within `Prog`
    /// (each instruction occupies two elements: opcode then argument).
    /// @tparam X Accumulator register carrying the current working value.
    /// @tparam Ram 256-byte addressable memory, as an integer_sequence.
    /// @tparam Out Output bytes emitted so far, as an integer_sequence.
    /// @tparam Prog Full bytecode program, as flat (opcode, argument) pairs.
    /// @return A stack_array of the emitted output bytes once the program
    /// counter runs past the end of `Prog`.
    /// @details Recurses one instruction per call until termination. Opcode
    /// meanings below (`arg` is the instruction's argument byte, `Ram` is
    /// memory, `X` is the accumulator): `G` loads `Ram[arg]` into `X`; `O`
    /// stores `X` into `Ram[arg]`; `R` resets `X` to 0, standing in for
    /// reading external input, which is not available at compile time; `B`
    /// jumps to instruction `arg` if `X` is zero, otherwise falls through to
    /// the next instruction; `I` adds the immediate `arg` into `X` modulo
    /// 256; `T` appends `X` to `Out`; `S` loads the immediate `arg` into `X`;
    /// `A` adds `Ram[arg]` into `X` modulo 256. The lowercase opcodes `g`,
    /// `o`, `r`, `b`, `i`, `t`, `s`, `a` mirror their uppercase counterparts
    /// but address memory indirectly through `Ram[arg]` (or, for `g` and `a`,
    /// doubly through `Ram[Ram[arg]]`); `s` additionally differs from `S` by
    /// XOR-ing `Ram[arg]` into `X` rather than loading an immediate.
    template < usize PC, u8 X, typename Ram, typename Out, typename Prog >
    consteval auto run_impl() 
    {
        if constexpr (PC + 1 >= arr_size_v<Prog>)
        { 
            return arr_collapse<Out>::value;
        }
        else 
        {
            constexpr char op = arr_elem_v<Prog, PC>;
            constexpr char arg = arr_elem_v<Prog, PC + 1>;

            if constexpr (op == 'G')
            {
                return run_impl<PC + 2, arr_elem_v<Ram, arg>, Ram, Out, Prog>();
            }
            else if constexpr (op == 'O')
            {
                return run_impl<PC + 2, X, arr_replace_t<Ram, arg, X>, Out, Prog>();
            }
            else if constexpr (op == 'R')
            {
                return run_impl<PC + 2, /*input*/ 0, Ram, Out, Prog>();
            }
            else if constexpr (op == 'B')
            {
                return run_impl<X ? PC + 2 : arg * 2, X, Ram, Out, Prog>();
            }
            else if constexpr (op == 'I')
            {
                return run_impl<PC + 2, (X + arg) & 0xFF, Ram, Out, Prog>();
            }
            else if constexpr (op == 'T') // output
            {
                return run_impl<PC + 2, X, Ram, arr_append_t<Out, X>, Prog>();
            }
            else if constexpr (op == 'S')
            {
                return run_impl<PC + 2, arg, Ram, Out, Prog>();
            }
            else if constexpr (op == 'A')
            {
                return run_impl<PC + 2, (X + arr_elem_v<Ram, arg>) & 0xFF, Ram, Out, Prog>();
            }
            else if constexpr (op == 'g')
            {
                return run_impl<PC + 2, arr_elem_v<Ram, arr_elem_v<Ram, arg>>, Ram, Out, Prog>();
            }
            else if constexpr (op == 'o')
            {
                return run_impl<PC + 2, X, arr_replace_t<Ram, arr_elem_v<Ram, arg>, X>, Out, Prog>();
            }
            else if constexpr (op == 'r')
            {
                return run_impl<PC + 2, X, arr_replace_t<Ram, arg, /*input*/ 0>, Out, Prog>();
            }
            else if constexpr (op == 'b')
            {
                return run_impl<X ? PC + 2 : arr_elem_v<Ram, arg> * 2, X, Ram, Out, Prog>();
            }
            else if constexpr (op == 'i')
            {
                return run_impl<PC + 2, X, arr_replace_t<Ram, arg, (arr_elem_v<Ram, arg> + X) & 0xFF>, Out, Prog>();
            }
            else if constexpr (op == 't') // output
            {
                return run_impl<PC + 2, X, Ram, arr_append_t<Out, arr_elem_v<Ram, arg>>, Prog>();
            }
            else if constexpr (op == 's')
            {
                return run_impl<PC + 2, X ^ arr_elem_v<Ram, arg>, Ram, Out, Prog>();
            }
            else if constexpr (op == 'a')
            {
                return run_impl<PC + 2, (X + arr_elem_v<Ram, arr_elem_v<Ram, arg>>) & 0xFF, Ram, Out, Prog>();
            }
            //if constexpr (op == ' ')
            //    return run_impl<PC + 2, X, Ram, Out, Prog>();
        }
    }

    /// @brief Compile-time tokenizer that turns Gorbitsa source text into a
    /// bytecode program and immediately executes it.
    /// @tparam Str Source character sequence being scanned.
    /// @tparam Parsing_command True when the next character starts a new
    /// opcode token rather than continuing a numeric argument or consuming a
    /// separating space.
    /// @tparam Tmp Decimal argument literal accumulated so far, modulo 256.
    /// @tparam I Current scan index into `Str`.
    /// @tparam c Bytecode emitted so far, as flat (opcode, argument) bytes.
    /// @return The result of executing the parsed program via run_impl.
    /// @details Source text is a space-separated sequence of tokens. `R` and
    /// `T` are bare tokens with no numeric argument, since both opcodes
    /// ignore their argument in run_impl, so it is always encoded as 0.
    /// Every other opcode character must be immediately followed by a
    /// decimal number in [0, 255] before the next separating space; digits
    /// accumulate into `Tmp` until a space closes the token, at which point
    /// `Tmp` is emitted as the argument byte. Recursion ends once the full
    /// source has been scanned, at which point the collected bytes are
    /// wrapped into a `Prog` sequence, paired with a zeroed 256-byte `Ram`,
    /// and passed to run_impl.
    template < typename Str, bool Parsing_command, u8 Tmp, usize I, u8... c >
    consteval auto run_parser() 
    {
        if constexpr (arr_size_v<Str> > I) 
        {
            if constexpr (arr_elem_v<Str, I> == 'R' || arr_elem_v<Str, I> == 'T') 
            {
    			if constexpr (arr_size_v<Str> > I + 1)
                {
    				static_assert(arr_elem_v<Str, I + 1> == ' ');
                }

    			return run_parser<Str, true, 0, I + 2, c..., arr_elem_v<Str, I>, 0>();
    		} 
            else 
            {
    			if constexpr (Parsing_command)
                {
    				return run_parser<Str, false, 0, I + 1, c..., arr_elem_v<Str, I>>();
                }
    			else if constexpr (arr_elem_v<Str, I> >= '0' && arr_elem_v<Str, I> <= '9')
                {
    				return run_parser<Str, false, (Tmp * 10 + arr_elem_v<Str, I> - '0') & 0xFF, I + 1, c...>();
                }
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
                0, 0, typename arr_makefill<char, 0, 256>::type, 
                integer_sequence<u8>, integer_sequence<u8, c...> >();
    	}
    }
}