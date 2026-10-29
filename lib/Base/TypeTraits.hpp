#pragma once

/// @file TypeTraits.hpp
/// @brief Project type traits and trait aliases.
/// @details Extends standard type traits with HackySTL naming and helpers for
/// cv/ref removal, decay, constructibility, convertibility, and type queries.

#include "Types.hpp"

namespace hsd
{
    /// @brief Wraps a compile-time constant of type \p T.
    /// @tparam T Type of the wrapped value.
    /// @tparam v Compile-time value being wrapped.
    /// @details Provides `type` (= \p T) and `value` (= \p v) members, mirroring `std::integral_constant`.
    template <typename T, T v>
    struct literal_constant
    {
        using type = T; ///< The wrapped value's type, \p T.
        static constexpr T value = v; ///< The wrapped compile-time value, \p v.
    };
    
    /// @brief `literal_constant` specialized to the boolean value `false`.
    using false_type = literal_constant<bool, false>;
    /// @brief `literal_constant` specialized to the boolean value `true`.
    using true_type = literal_constant<bool, true>;

    /// @brief Primary template; provides no `type` member unless the condition holds.
    /// @details SFINAE gate: instantiating `enable_if<false, T>::type` is ill-formed.
    template <bool, typename>
    struct enable_if
    {};

    /// @brief Specialization enabled when the condition is `true`.
    /// @tparam T Type exposed as `type` when the condition holds.
    template <typename T>
    struct enable_if<true, T>
    {
        using type = T; ///< \p T, exposed only when the condition is `true`.
    };

    /// @brief Primary template; provides no `type` member unless the condition is `false`.
    /// @details Inverse SFINAE gate of `enable_if`: instantiating `disable_if<true, T>::type` is ill-formed.
    template <bool, typename>
    struct disable_if
    {};

    /// @brief Specialization enabled when the condition is `false`.
    /// @tparam T Type exposed as `type` when the condition does not hold.
    template <typename T>
    struct disable_if<false, T>
    {
        using type = T; ///< \p T, exposed only when the condition is `false`.
    };

    /// @brief Logical AND across two or more trait types.
    /// @tparam Cond Trait types, each exposing a static boolean `value`.
    /// @details `value` is `true` only if every `Cond::value` is `true`; requires at least two traits.
    template <typename... Cond> requires(sizeof...(Cond) > 1)
    struct conjunction
    {
        static constexpr bool value = (Cond::value && ...); ///< `true` iff every `Cond::value` is `true`.
    };

    /// @brief Logical OR across two or more trait types.
    /// @tparam Cond Trait types, each exposing a static boolean `value`.
    /// @details `value` is `true` if any `Cond::value` is `true`; requires at least two traits.
    template <typename... Cond> requires(sizeof...(Cond) > 1)
    struct disjunction
    {
        static constexpr bool value = (Cond::value || ...); ///< `true` iff any `Cond::value` is `true`.
    };

    /// @brief Logical NOT of a single trait type.
    /// @tparam Cond Trait type exposing a static boolean `value`.
    /// @details `value` holds the negation of `Cond::value`.
    template <typename Cond>
    struct negation
    {
        static constexpr bool value = !Cond::value; ///< The logical negation of `Cond::value`.
    };

    /// @brief Primary template selecting \p IfTrue as `type` when \p Cond is `true`.
    /// @tparam Cond Compile-time boolean selector.
    /// @tparam IfTrue Type selected when \p Cond is `true`.
    /// @tparam IfFalse Type selected when \p Cond is `false`.
    template < bool Cond, typename IfTrue, typename IfFalse >
    struct conditional
    {
        using type = IfTrue; ///< \p IfTrue, selected when \p Cond is `true`.
    };
    
    /// @brief Specialization selecting \p IfFalse as `type` when \p Cond is `false`.
    template < typename IfTrue, typename IfFalse >
    struct conditional< false, IfTrue, IfFalse >
    {
        using type = IfFalse; ///< \p IfFalse, selected when \p Cond is `false`.
    };

    /// @brief Predicate: detects array types.
    /// @tparam T Type queried, unmatched by any array form.
    /// @details Primary template; `value` is `false` for non-array types.
    template <typename>
    struct is_array
        : public false_type 
    {};

    /// @brief Specialization matching bounded array types `T[size]`.
    /// @tparam T Element type.
    /// @tparam size Array extent.
    template <typename T, usize size>
    struct is_array<T[size]>
        : public true_type 
    {};

    /// @brief Specialization matching unbounded array types `T[]`.
    /// @tparam T Element type.
    template <typename T>
    struct is_array<T[]>
        : public true_type 
    {};

    /// @brief Predicate: detects whether two types are identical.
    /// @details Primary template; `value` is `false` for distinct types.
    template <typename, typename>
    struct is_same
        : public false_type
    {};
    
    /// @brief Specialization matching when both type arguments are the same type \p T.
    /// @tparam T Common type.
    template <typename T>
    struct is_same<T, T>
        : public true_type
    {};

    /// @brief Predicate: detects top-level `const`-qualified types.
    /// @details Primary template; `value` is `false` for unqualified types.
    template <typename> 
    struct is_const  
        : public false_type
    {};

    /// @brief Specialization matching `const`-qualified types.
    /// @tparam T Underlying (unqualified) type.
    template <typename T> 
    struct is_const<const T>
        : public true_type
    {};

    /// @brief Adds a top-level `const` qualifier to \p T.
    /// @tparam T Type to qualify.
    /// @details `type` is `const T`.
    template <typename T> 
    struct add_const
    {
        using type = const T; ///< \p T with a top-level `const` qualifier added.
    };

    /// @brief Specialization avoiding a redundant `const` when \p T is already `const`-qualified.
    template <typename T> 
    struct add_const<const T>
    {
        using type = const T; ///< \p T, unchanged since it is already `const`-qualified.
    };

    /// @brief Predicate: detects lvalue reference types.
    /// @details Primary template; `value` is `false` for non-reference and rvalue-reference types.
    template <typename>
    struct is_lvalue_reference
        : public false_type
    {};

    /// @brief Specialization matching lvalue reference types `T&`.
    /// @tparam T Referenced type.
    template <typename T>
    struct is_lvalue_reference<T&>
        : public true_type
    {};

    /// @brief Adds an lvalue reference to \p T.
    /// @tparam T Type to reference.
    /// @details `type` is `T&`.
    template <typename T>
    struct add_lvalue_reference
    {
        using type = T&; ///< An lvalue reference to \p T.
    };

    /// @brief Specialization collapsing `T&` to a single lvalue reference.
    template <typename T>
    struct add_lvalue_reference<T&>
    {
        using type = T&; ///< An lvalue reference to \p T (already a reference).
    };

    /// @brief Predicate: detects rvalue reference types.
    /// @details Primary template; `value` is `false` for non-reference and lvalue-reference types.
    template <typename>
    struct is_rvalue_reference
        : public false_type
    {};

    /// @brief Specialization matching rvalue reference types `T&&`.
    /// @tparam T Referenced type.
    template <typename T>
    struct is_rvalue_reference<T&&>
        : public true_type 
    {};

    /// @brief Adds an rvalue reference to \p T.
    /// @tparam T Type to reference.
    /// @details `type` is `T&&`.
    template <typename T>
    struct add_rvalue_reference
    {
        using type = T&&; ///< An rvalue reference to \p T.
    };

    /// @brief Specialization collapsing `T&&` to a single rvalue reference.
    template <typename T>
    struct add_rvalue_reference<T&&>
    {
        using type = T&&; ///< An rvalue reference to \p T (already a reference).
    };

    /// @brief Predicate: detects any reference type (lvalue or rvalue).
    /// @tparam T Type queried.
    /// @details `value` is `true` if `T` is an lvalue or rvalue reference.
    template <typename T>
    struct is_reference
        : public disjunction<
            is_rvalue_reference<T>, 
            is_lvalue_reference<T> >
    {};

    /// @brief Implementation details backing the cv-unqualified public trait aliases below.
    /// @details Each trait here expects an already cv-unqualified type; the public aliases
    /// (outside this namespace) strip cv-qualification via `remove_cv` before delegating here.
    namespace helper
    {
        /// @brief Predicate: detects the unqualified type `void`.
        /// @details Primary template; `value` is `false` for any type other than `void`.
        template <typename>
        struct is_void
            : public false_type
        {};
        
        /// @brief Specialization matching `void` exactly.
        template <>
        struct is_void<void>
            : public true_type
        {};

        /// @brief Predicate: detects pointer types.
        /// @details Primary template; `value` is `false` for non-pointer types.
        template <typename>
        struct is_pointer
            : public false_type
        {};
    
        /// @brief Specialization matching pointer types `T*`.
        /// @tparam T Pointee type.
        template <typename T>
        struct is_pointer<T*>
            : public true_type
        {};

        /// @brief Predicate: detects built-in integral types.
        /// @details Primary template; `value` is `false` unless matched by an explicit
        /// specialization for one of the fundamental integer/character/boolean types below.
        template <typename>
        struct is_integral
            : public false_type
        {};

        /// @brief Specialization matching `bool`.
        template <>
        struct is_integral<bool>
            : public true_type
        {};

        /// @brief Specialization matching `char`.
        template <>
        struct is_integral<char>
            : public true_type
        {};
        
        /// @brief Specialization matching `char8` (UTF-8 code unit type).
        template <>
        struct is_integral<char8>
            : public true_type
        {};

        /// @brief Specialization matching `char16` (UTF-16 code unit type).
        template <>
        struct is_integral<char16>
            : public true_type
        {};

        /// @brief Specialization matching `char32` (UTF-32 code unit type).
        template <>
        struct is_integral<char32>
            : public true_type
        {};

        /// @brief Specialization matching `wchar`.
        template <>
        struct is_integral<wchar>
            : public true_type
        {};

        /// @brief Specialization matching the signed 8-bit integer type `i8`.
        template <>
        struct is_integral<i8>
            : public true_type
        {};

        /// @brief Specialization matching the signed 16-bit integer type `i16`.
        template <>
        struct is_integral<i16>
            : public true_type
        {};

        /// @brief Specialization matching the signed 32-bit integer type `i32`.
        template <>
        struct is_integral<i32>
            : public true_type
        {};

        /// @brief Specialization matching the signed 64-bit integer type `i64`.
        template <>
        struct is_integral<i64>
            : public true_type
        {};

        #if defined(HSD_COMPILER_GCC) && !defined(HSD_LIBRARY_INT128)

        /// @brief Specialization matching the signed 128-bit integer type `i128` (GCC extension).
        template <>
        struct is_integral<i128>
            : public true_type
        {};

        #endif

    #if !defined(HSD_COMPILER_GCC) || defined(HSD_LIBRARY_INT128)
        /// @brief Specialization matching the library's signed 128-bit integer type.
        template <>
        struct is_integral<int128_detail::basic_int128<true>>
            : public true_type
        {};
    #endif

        /// @brief Specialization matching `long`.
        template <>
        struct is_integral<long>
            : public true_type
        {};

        /// @brief Specialization matching the unsigned character type `uchar`.
        template <>
        struct is_integral<uchar>
            : public true_type
        {};

        /// @brief Specialization matching the unsigned 16-bit integer type `u16`.
        template <>
        struct is_integral<u16>
            : public true_type
        {};

        /// @brief Specialization matching the unsigned 32-bit integer type `u32`.
        template <>
        struct is_integral<u32>
            : public true_type
        {};

        /// @brief Specialization matching the unsigned 64-bit integer type `u64`.
        template <>
        struct is_integral<u64>
            : public true_type
        {};

        #if defined(HSD_COMPILER_GCC) && !defined(HSD_LIBRARY_INT128)

        /// @brief Specialization matching the unsigned 128-bit integer type `u128` (GCC extension).
        template <>
        struct is_integral<u128>
            : public true_type
        {};

        #endif

    #if !defined(HSD_COMPILER_GCC) || defined(HSD_LIBRARY_INT128)
        /// @brief Specialization matching the library's unsigned 128-bit integer type.
        template <>
        struct is_integral<int128_detail::basic_int128<false>>
            : public true_type
        {};
    #endif

        /// @brief Specialization matching `unsigned long`.
        template <>
        struct is_integral<ulong>
            : public true_type
        {};

        /// @brief Predicate: detects built-in floating-point types.
        /// @details Primary template; `value` is `false` unless matched by an explicit
        /// specialization for `f32`, `f64`, or `f128`.
        template <typename>
        struct is_floating_point
            : public false_type
        {};

        /// @brief Specialization matching the 32-bit floating-point type `f32`.
        template <>
        struct is_floating_point<f32>
            : public true_type
        {};

        /// @brief Specialization matching the 64-bit floating-point type `f64`.
        template <>
        struct is_floating_point<f64>
            : public true_type
        {};

        /// @brief Specialization matching the 128-bit floating-point type `f128`.
        template <>
        struct is_floating_point<f128>
            : public true_type
        {};

        /// @brief Predicate: detects any arithmetic (integral or floating-point) type.
        /// @tparam T Type queried.
        /// @details `value` is `true` if `T` satisfies `is_integral` or `is_floating_point`.
        template <typename T>
        struct is_number
            : disjunction< is_integral<T>, is_floating_point<T> >
        {};

        /// @brief Predicate: detects signed arithmetic types.
        /// @tparam T Type queried.
        /// @details Primary/false branch used when `T` is not a number; `value` is `false`.
        template < typename T, bool = is_number<T>::value >
        struct is_signed : public false_type 
        {};

        /// @brief Specialization used when `T` is a number: `value` is `true` iff `T(-1) < T(0)`.
        /// @tparam T Arithmetic type queried.
        template <typename T>
        struct is_signed<T, true>
            : public literal_constant< 
                bool, is_number<T>::value && (T(-1) < T(0)) 
            >
        {};

        /// @brief Predicate: detects unsigned arithmetic types.
        /// @tparam T Type queried.
        /// @details `value` is `true` iff `T` is a number and `T(-1) > T(0)`.
        template <typename T>
        struct is_unsigned
            : public literal_constant< 
                bool, is_number<T>::value && (T(-1) > T(0)) 
            >
        {};
    }

    /// @brief Strips any reference qualification from \p T.
    /// @tparam T Type to unwrap.
    /// @details Primary template; `type` is `T` unchanged (already unreferenced).
    template <typename T>
    struct remove_reference
    {
        using type = T; ///< \p T, already unreferenced.
    };

    /// @brief Specialization stripping an lvalue reference `T&`.
    template <typename T>
    struct remove_reference<T&>
    {
        using type = T; ///< \p T with the lvalue reference removed.
    };

    /// @brief Specialization stripping an rvalue reference `T&&`.
    template <typename T>
    struct remove_reference<T&&>
    {
        using type = T; ///< \p T with the rvalue reference removed.
    };

    /// @brief Strips array-ness from \p T, yielding its element type.
    /// @tparam T Type to unwrap.
    /// @details Primary template; `type` is `T` unchanged (not an array).
    template <typename T>
    struct remove_array
    {
        using type = T; ///< \p T, unchanged since it is not an array.
    };

    /// @brief Specialization stripping an unbounded array `T[]`.
    template <typename T>
    struct remove_array<T[]>
    {
        using type = T; ///< The element type of the unbounded array.
    };

    /// @brief Specialization stripping a bounded array `T[N]`.
    /// @tparam N Array extent, discarded from the result.
    template <typename T, usize N>
    struct remove_array<T[N]>
    {
        using type = T; ///< The element type of the bounded array.
    };

    /// @brief Strips top-level `const`/`volatile` qualification from \p T.
    /// @tparam T Type to unwrap.
    /// @details Primary template; `type` is `T` unchanged (unqualified).
    template <typename T> 
    struct remove_cv                   
    {
        using type = T; ///< \p T, unchanged since it is unqualified.
    };

    /// @brief Specialization stripping a `const` qualifier.
    template <typename T> 
    struct remove_cv<const T>          
    {
        using type = T; ///< \p T with the `const` qualifier removed.
    };

    /// @brief Specialization stripping a `volatile` qualifier.
    template <typename T> 
    struct remove_cv<volatile T>       
    {
        using type = T; ///< \p T with the `volatile` qualifier removed.
    };

    /// @brief Specialization stripping combined `const volatile` qualifiers.
    template <typename T> 
    struct remove_cv<const volatile T> 
    {
        using type = T; ///< \p T with both `const` and `volatile` qualifiers removed.
    };

    /// @brief Adds top-level `const volatile` qualification to \p T.
    /// @tparam T Type to qualify.
    /// @details `type` is `const volatile T`.
    template <typename T> 
    struct add_cv                   
    {
        using type = const volatile T; ///< \p T qualified with both `const` and `volatile`.
    };

    /// @brief Specialization avoiding a redundant `const` when \p T is already `const`-qualified.
    template <typename T> 
    struct add_cv<const T>          
    {
        using type = const volatile T; ///< \p T with `volatile` added to the existing `const`.
    };

    /// @brief Specialization avoiding a redundant `volatile` when \p T is already `volatile`-qualified.
    template <typename T> 
    struct add_cv<volatile T>       
    {
        using type = const volatile T; ///< \p T with `const` added to the existing `volatile`.
    };

    /// @brief Specialization avoiding redundant qualification when \p T is already `const volatile`.
    template <typename T> 
    struct add_cv<const volatile T> 
    {
        using type = const volatile T; ///< \p T, already `const volatile`-qualified.
    };
    
    /// @brief Strips a top-level `const` qualifier from \p T.
    /// @tparam T Type to unwrap.
    /// @details Primary template; `type` is `T` unchanged (not `const`-qualified).
    template <typename T> 
    struct remove_const                
    {
        using type = T; ///< \p T, unchanged since it is not `const`-qualified.
    };

    /// @brief Specialization stripping a `const` qualifier.
    template <typename T> 
    struct remove_const<const T>       
    {
        using type = T; ///< \p T with the `const` qualifier removed.
    };

    /// @brief Strips a top-level `volatile` qualifier from \p T.
    /// @tparam T Type to unwrap.
    /// @details Primary template; `type` is `T` unchanged (not `volatile`-qualified).
    template <typename T> 
    struct remove_volatile             
    {
        using type = T; ///< \p T, unchanged since it is not `volatile`-qualified.
    };

    /// @brief Specialization stripping a `volatile` qualifier.
    template <typename T> 
    struct remove_volatile<volatile T> 
    {
        using type = T; ///< \p T with the `volatile` qualifier removed.
    };

    /// @brief Adds a top-level `volatile` qualifier to \p T.
    /// @tparam T Type to qualify.
    /// @details `type` is `volatile T`.
    template <typename T> 
    struct add_volatile             
    {
        using type = volatile T; ///< \p T qualified with `volatile`.
    };

    /// @brief Specialization avoiding a redundant `volatile` when \p T is already `volatile`-qualified.
    template <typename T> 
    struct add_volatile<volatile T> 
    {
        using type = volatile T; ///< \p T, already `volatile`-qualified.
    };

    /// @brief Function-type detection for C-style variadic signatures `Ret(Args..., ...)`.
    namespace va_args
    {
        /// @brief Detects unqualified, non-ref-qualified, non-noexcept variadic function types.
        namespace regular
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type
            {};
    
            /// @brief Specialization matching `Ret(Args..., ...)`.
            /// @tparam Ret Return type.
            /// @tparam Args Fixed (non-variadic) parameter types preceding the `...`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...)>
                : true_type 
            {};
        } // namespace regular

        /// @brief Detects cv-qualified (non-ref, non-noexcept) variadic function types.
        namespace cv
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) volatile`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) volatile>
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const volatile`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const volatile>
                : true_type
            {};
        } // namespace cv

        /// @brief Detects ref-qualified (with optional cv-qualification, non-noexcept) variadic function types.
        namespace ref
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type
            {};

            /// @brief Specialization matching `Ret(Args..., ...) &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) volatile &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) volatile &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const volatile &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const volatile &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) &&> 
                : true_type 
            {};
            
            /// @brief Specialization matching `Ret(Args..., ...) const &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const &&> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) volatile &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) volatile &&> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const volatile &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const volatile &&> 
                : true_type 
            {};
        } // namespace ref

        /// @brief Detects `noexcept`-qualified variadic function types (with optional cv/ref qualification).
        namespace noexp
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type
            {};

            /// @brief Specialization matching `Ret(Args..., ...) noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) volatile noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) volatile noexcept>
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const volatile noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const volatile noexcept>
                : true_type
            {};

            /// @brief Specialization matching `Ret(Args..., ...) & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) volatile & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) volatile & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const volatile & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const volatile & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) && noexcept> 
                : true_type 
            {};
            
            /// @brief Specialization matching `Ret(Args..., ...) const && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const && noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) volatile && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) volatile && noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args..., ...) const volatile && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args..., ...) const volatile && noexcept> 
                : true_type 
            {};
        } // namespace noexp

        /// @brief Predicate: detects any C-style variadic function type.
        /// @tparam T Type queried.
        /// @details `value` is `true` if `T` matches any qualifier/ref/noexcept combination
        /// of a `Ret(Args..., ...)` signature detected by the `regular`/`cv`/`ref`/`noexp` namespaces.
        template <typename T>
        struct is_va_args_function
            : disjunction<
                regular::is_function<T>, cv::is_function<T>, 
                ref::is_function<T>, noexp::is_function<T> >
        {};
        
    } // namespace va_args


    /// @brief Function-type detection for regular (non-variadic) signatures `Ret(Args...)`.
    namespace reg_args
    {
        /// @brief Detects unqualified, non-ref-qualified, non-noexcept function types.
        namespace regular
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type 
            {};
    
            /// @brief Specialization matching `Ret(Args...)`.
            /// @tparam Ret Return type.
            /// @tparam Args Parameter types.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...)> 
                : true_type 
            {};
        } // namespace regular
        
        /// @brief Detects cv-qualified (non-ref, non-noexcept) function types.
        namespace cv
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) volatile`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) volatile>
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const volatile`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const volatile>
                : true_type
            {};
        } // namespace cv

        /// @brief Detects ref-qualified (with optional cv-qualification, non-noexcept) function types.
        namespace ref
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type 
            {};

            /// @brief Specialization matching `Ret(Args...) &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) volatile &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) volatile &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const volatile &`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const volatile &> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) &&> 
                : true_type 
            {};
            
            /// @brief Specialization matching `Ret(Args...) const &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const &&> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) volatile &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) volatile &&> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const volatile &&`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const volatile &&> 
                : true_type 
            {};
        } // namespace ref
        
        /// @brief Detects `noexcept`-qualified function types (with optional cv/ref qualification).
        namespace noexp
        {
            /// @brief Predicate primary template; `value` is `false` for non-function types.
            template <typename>
            struct is_function 
                : false_type 
            {};

            /// @brief Specialization matching `Ret(Args...) noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) volatile noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) volatile noexcept>
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const volatile noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const volatile noexcept>
                : true_type
            {};

            /// @brief Specialization matching `Ret(Args...) & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) volatile & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) volatile & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const volatile & noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const volatile & noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) && noexcept> 
                : true_type 
            {};
            
            /// @brief Specialization matching `Ret(Args...) const && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const && noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) volatile && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) volatile && noexcept> 
                : true_type 
            {};

            /// @brief Specialization matching `Ret(Args...) const volatile && noexcept`.
            template < typename Ret, typename... Args >
            struct is_function<Ret(Args...) const volatile && noexcept> 
                : true_type 
            {};
        } // namespace noexp

        /// @brief Predicate: detects any regular (non-variadic) function type.
        /// @tparam T Type queried.
        /// @details `value` is `true` if `T` matches any qualifier/ref/noexcept combination
        /// of a `Ret(Args...)` signature detected by the `regular`/`cv`/`ref`/`noexp` namespaces.
        template <typename T>
        struct is_reg_args_function
            : disjunction<
                regular::is_function<T>, cv::is_function<T>, 
                ref::is_function<T>, noexp::is_function<T> >
        {};
    } // namespace reg_args

    /// @brief Predicate: detects any function type (regular or C-style variadic).
    /// @tparam T Type queried.
    /// @details `value` is `true` if `T` is matched by `reg_args::is_reg_args_function`
    /// or `va_args::is_va_args_function`.
    template <typename T>
    struct is_function
        : disjunction<
            reg_args::is_reg_args_function<T>, 
            va_args::is_va_args_function<T> >
    {};

    /// @brief Predicate: detects object types (not `void`, a reference, or a function).
    /// @tparam T Type queried.
    /// @details `value` is `true` unless `T` is a function type, a reference type, or
    /// (after removing cv-qualification) `void`.
    template<typename T>
    struct is_object
        : public negation< 
            disjunction< is_function<T>, is_reference<T>,
            helper::is_void< typename remove_cv<T>::type > > 
        >
    {};


    /// @brief SFINAE-based helpers backing `add_pointer` and the free `declval` function.
    namespace sfinae
    {
        /// @brief Identity wrapper used to carry a type through `decltype` in overload resolution.
        /// @tparam T Wrapped type, exposed unchanged as `type`.
        template <typename T>
        struct type_identity
        {
            using type = T; ///< The wrapped type, \p T, unchanged.
        };

        /// @brief Overload selected when `remove_reference<T>::type*` is well-formed.
        /// @tparam T Type being pointer-adjusted.
        /// @details Returns `type_identity` of a pointer to the reference-stripped type;
        /// used via `decltype` to implement `add_pointer` for referenceable/object types.
        /// @return `type_identity< typename remove_reference<T>::type* >`.
        template <typename T>
        constexpr auto try_add_pointer(int) -> type_identity< typename remove_reference<T>::type* >;
        /// @brief Fallback overload selected when the pointer form above is ill-formed.
        /// @tparam T Type being pointer-adjusted (left unchanged).
        /// @return `type_identity<T>`.
        template <typename T>
        constexpr auto try_add_pointer(...) -> type_identity<T>;

        /// @brief Overload producing an rvalue reference to \p T for use in unevaluated contexts.
        /// @tparam T Type to reference.
        /// @return An rvalue reference to \p T.
        template <typename T> 
        T&& declval(int);

        /// @brief Fallback overload producing \p T by value when the rvalue-reference form is unsuitable.
        /// @tparam T Type to produce.
        /// @return A value of type \p T.
        template <typename T>
        T declval(long);
    } // namespace sfinae

    /// @brief Adds a pointer to \p T, or leaves \p T unchanged if pointer formation is ill-formed.
    /// @tparam T Type to point to.
    /// @details Mirrors `std::add_pointer`; `type` is `remove_reference<T>::type*` when valid,
    /// otherwise `T` itself (e.g. for function types with cv/ref/noexcept qualifiers).
    template <typename T>
    struct add_pointer
        : decltype(sfinae::try_add_pointer<T>(0)) 
    {};

    /// @brief Strips one level of pointer indirection from \p T.
    /// @tparam T Type to unwrap.
    /// @details Primary template; `type` is `T` unchanged (not a pointer).
    template <typename T>
    struct remove_pointer
    {
        using type = T; ///< \p T, unchanged since it is not a pointer.
    };

    /// @brief Specialization stripping an unqualified pointer `T*`.
    template <typename T>
    struct remove_pointer<T*>
    {
        using type = T; ///< The pointee type of `T*`.
    };

    /// @brief Specialization stripping a `const`-qualified pointer `T* const`.
    template <typename T>
    struct remove_pointer<T* const>
    {
        using type = T; ///< The pointee type of `T* const`.
    };

    /// @brief Specialization stripping a `volatile`-qualified pointer `T* volatile`.
    template <typename T>
    struct remove_pointer<T* volatile>
    {
        using type = T; ///< The pointee type of `T* volatile`.
    };

    /// @brief Specialization stripping a `const volatile`-qualified pointer `T* const volatile`.
    template <typename T>
    struct remove_pointer<T* const volatile>
    {
        using type = T; ///< The pointee type of `T* const volatile`.
    };

    /// @brief Computes the decayed form of \p T, mirroring `std::decay`.
    /// @tparam T Type to decay.
    /// @details References are stripped first; arrays decay to element pointers, function types
    /// decay to function pointers, and remaining cv-qualification is removed otherwise.
    template <typename T>
    struct decay 
    {
    private:
        using U = typename remove_reference<T>::type; ///< \p T with any reference stripped.

    public:
        using type = typename conditional< 
            is_array<U>::value,
            typename remove_array<U>::type*,
            typename conditional< 
                is_function<U>::value,
                typename add_pointer<U>::type,
                typename remove_cv<U>::type
            >::type
        >::type; ///< The decayed type, per the array/function/cv-qualification rules above.
    };

    /// @brief Produces a reference to \p T for use in unevaluated contexts (e.g. `decltype`).
    /// @tparam T Type to reference.
    /// @details Has no definition; must only appear in unevaluated expressions.
    /// @return A reference to \p T (or \p T by value if referencing is ill-formed).
    template <typename T>
    auto declval() -> decltype(sfinae::declval<T>(0));

    /// @brief Predicate: detects whether an \p From value converts implicitly to \p To.
    /// @tparam From Source type of the conversion.
    /// @tparam To Target type of the conversion.
    /// @details `value` is `true` iff an expression of type \p From can be implicitly
    /// converted to \p To, matching `std::is_convertible`. Backed by the compiler's
    /// conversion intrinsic, so no standard library header is needed.
    #if defined(HSD_COMPILER_MSVC)
    template <typename From, typename To>
    struct is_convertible
        : public literal_constant< bool, __is_convertible_to(From, To) >
    {};
    #else
    template <typename From, typename To>
    struct is_convertible
        : public literal_constant< bool, __is_convertible(From, To) >
    {};
    #endif

    /// @brief Alias yielding \p Value only when \p Condition::value is `true`.
    /// @tparam Condition Trait type exposing a static boolean `value`.
    /// @tparam Value Type produced when the condition holds.
    template < typename Condition, typename Value >
    using ResolvedType = typename enable_if< Condition::value, Value >::type;

    /// @brief Alias yielding \p Value only when \p Condition::value is `false`.
    /// @tparam Condition Trait type exposing a static boolean `value`.
    /// @tparam Value Type produced when the condition does not hold.
    template < typename Condition, typename Value >
    using UnResolvedType = typename disable_if< Condition::value, Value >::type;

    /// @brief Alias yielding \p Result only when \p From is convertible to \p To.
    /// @tparam From Source type of the convertibility check.
    /// @tparam To Target type of the convertibility check.
    /// @tparam Result Type produced when the conversion is valid.
    template < typename From, typename To, typename Result >
    using EnableIfConvertible = ResolvedType< is_convertible<From, To>, Result >;

    /// @brief Alias yielding \p Result only when \p From is not convertible to \p To.
    /// @tparam From Source type of the convertibility check.
    /// @tparam To Target type of the convertibility check.
    /// @tparam Result Type produced when the conversion is invalid.
    template < typename From, typename To, typename Result >
    using DisableIfConvertible = UnResolvedType< is_convertible<From, To>, Result >;

    /// @brief Alias yielding \p Result only when \p First and \p Second are the same type.
    /// @tparam First First type to compare.
    /// @tparam Second Second type to compare.
    /// @tparam Result Type produced when the types match.
    template < typename First, typename Second, typename Result >
    using EnableIfSame = ResolvedType< is_same<First, Second>, Result >;

    /// @brief Alias yielding \p Result when \p First and \p Second are the same type.
    /// @tparam First First type to compare.
    /// @tparam Second Second type to compare.
    /// @tparam Result Type produced when the types match.
    /// @note Despite the name, this currently uses `ResolvedType` (same condition polarity as
    /// `EnableIfSame`), not the negated `UnResolvedType`; preserved as-is for behavior compatibility.
    template < typename First, typename Second, typename Result >
    using DisableIfSame = ResolvedType< is_same<First, Second>, Result >;

    /// @brief Public integral-type predicate alias; strips cv-qualification before delegating to `helper::is_integral`.
    /// @tparam T Type queried.
    template <typename T>
    using is_integral = helper::is_integral< typename remove_cv<T>::type >;

    /// @brief Public floating-point predicate alias; strips cv-qualification before delegating to `helper::is_floating_point`.
    /// @tparam T Type queried.
    template <typename T>
    using is_floating_point = helper::is_floating_point< typename remove_cv<T>::type >;

    /// @brief Public arithmetic-type predicate alias; strips cv-qualification before delegating to `helper::is_number`.
    /// @tparam T Type queried.
    template <typename T>
    using is_number = helper::is_number< typename remove_cv<T>::type >;

    /// @brief Public signed-arithmetic predicate alias; strips cv-qualification before delegating to `helper::is_signed`.
    /// @tparam T Type queried.
    template <typename T>
    using is_signed = helper::is_signed< typename remove_cv<T>::type >;

    /// @brief Public unsigned-arithmetic predicate alias; strips cv-qualification before delegating to `helper::is_unsigned`.
    /// @tparam T Type queried.
    template <typename T>
    using is_unsigned = helper::is_unsigned< typename remove_cv<T>::type >;

    /// @brief Public `void`-type predicate alias; strips cv-qualification before delegating to `helper::is_void`.
    /// @tparam T Type queried.
    template <typename T>
    using is_void = helper::is_void< typename remove_cv<T>::type >;

    /// @brief Public pointer-type predicate alias; strips cv-qualification before delegating to `helper::is_pointer`.
    /// @tparam T Type queried.
    template <typename T>
    using is_pointer = helper::is_pointer< typename remove_cv<T>::type >;

    /// @brief Predicate: detects class types, meaning structs and classes.
    /// @tparam T Type queried.
    /// @details `value` is `true` iff `T` is a class or struct type. Unions, enums
    /// and scalars are not classes, matching `std::is_class`. Backed by the
    /// compiler's intrinsic, so no standard library header is needed.
    template <typename T>
    struct is_class
        : public literal_constant< bool, __is_class(T) >
    {};

    /// @brief Predicate: detects types the compiler treats as trivially copyable.
    /// @tparam T Type queried.
    /// @details `value` is `true` iff copies of `T` may be made by copying its
    /// object representation. Unlike the predicates above, cv-qualifiers are
    /// kept, matching `std::is_trivially_copyable`.
    template <typename T>
    struct is_trivially_copyable
        : public literal_constant< bool, __is_trivially_copyable(T) >
    {};

    #if defined(__has_builtin)
        #if __has_builtin(__is_trivially_destructible)
            /// @brief Compiler intrinsic behind `is_trivially_destructible`.
            #define HSD_IS_TRIVIALLY_DESTRUCTIBLE(T) __is_trivially_destructible(T)
        #else
            /// @brief GCC 15 and older have no `__is_trivially_destructible`; the older
            /// builtin gives the same answer there.
            #define HSD_IS_TRIVIALLY_DESTRUCTIBLE(T) __has_trivial_destructor(T)
        #endif
    #else
        #define HSD_IS_TRIVIALLY_DESTRUCTIBLE(T) __is_trivially_destructible(T)
    #endif

    /// @brief Predicate: detects types whose destructor does nothing.
    /// @tparam T Type queried.
    /// @details `value` is `true` iff destroying a `T` needs no call to its
    /// destructor, matching `std::is_trivially_destructible`. Backed by the
    /// compiler's intrinsic, so no standard library header is needed.
    template <typename T>
    struct is_trivially_destructible
        : public literal_constant< bool, HSD_IS_TRIVIALLY_DESTRUCTIBLE(T) >
    {};

    #undef HSD_IS_TRIVIALLY_DESTRUCTIBLE

    /// @brief Predicate: detects whether `T` can be constructed from `Args...` without throwing.
    /// @tparam T Type constructed.
    /// @tparam Args Argument types passed to the constructor.
    /// @details `value` is `true` iff `T` can be constructed from arguments of
    /// types `Args...` and that construction is declared `noexcept`, matching
    /// `std::is_nothrow_constructible`. Backed by the compiler's intrinsic, so no
    /// standard library header is needed.
    template <typename T, typename... Args>
    struct is_nothrow_constructible
        : public literal_constant< bool, __is_nothrow_constructible(T, Args...) >
    {};

    /// @brief Convenience alias for `conditional<Cond, IfTrue, IfFalse>::type`.
    /// @tparam Cond Compile-time boolean selector.
    /// @tparam IfTrue Type selected when \p Cond is `true`.
    /// @tparam IfFalse Type selected when \p Cond is `false`.
    template < bool Cond, typename IfTrue, typename IfFalse >
    using conditional_t = typename conditional< Cond, IfTrue, IfFalse >::type;

    /// @brief Convenience alias for `remove_array<T>::type`.
    /// @tparam T Type to unwrap.
    template <typename T>
    using remove_array_t = typename remove_array<T>::type;

    /// @brief Convenience alias for `remove_cv<T>::type`.
    /// @tparam T Type to unwrap.
    template <typename T>
    using remove_cv_t = typename remove_cv<T>::type;

    /// @brief Convenience alias combining `remove_cv` and `remove_reference` on \p T.
    /// @tparam T Type to unwrap.
    /// @details Strips references first, then cv-qualification, mirroring `std::remove_cvref_t`.
    template <typename T>
    using remove_cvref_t = typename remove_cv<typename remove_reference<T>::type>::type;

    /// @brief Convenience alias for `remove_const<T>::type`.
    /// @tparam T Type to unwrap.
    template <typename T>
    using remove_const_t = typename remove_const<T>::type;

    /// @brief Convenience alias for `remove_volatile<T>::type`.
    /// @tparam T Type to unwrap.
    template <typename T>
    using remove_volatile_t = typename remove_volatile<T>::type;

    /// @brief Convenience alias for `remove_reference<T>::type`.
    /// @tparam T Type to unwrap.
    template <typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    /// @brief Convenience alias for `remove_pointer<T>::type`.
    /// @tparam T Type to unwrap.
    template <typename T>
    using remove_pointer_t = typename remove_pointer<T>::type;

    /// @brief Convenience alias for `add_pointer<T>::type`.
    /// @tparam T Type to point to.
    template <typename T>
    using add_pointer_t = typename add_pointer<T>::type;

    /// @brief Convenience alias for `add_const<T>::type`.
    /// @tparam T Type to qualify.
    template <typename T>
    using add_const_t = typename add_const<T>::type;

    /// @brief Convenience alias for `add_cv<T>::type`.
    /// @tparam T Type to qualify.
    template <typename T>
    using add_cv_t = typename add_cv<T>::type;

    /// @brief Convenience alias for `add_rvalue_reference<T>::type`.
    /// @tparam T Type to reference.
    template <typename T>
    using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

    /// @brief Convenience alias for `add_lvalue_reference<T>::type`.
    /// @tparam T Type to reference.
    template <typename T>
    using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

    /// @brief Convenience alias for `decay<T>::type`.
    /// @tparam T Type to decay.
    template <typename T>
    using decay_t = typename decay<T>::type;

    /// @brief Convenience alias for `enable_if<Cond, T>::type`, defaulting \p T to `void`.
    /// @tparam Cond Compile-time boolean gate.
    /// @tparam T Type exposed when \p Cond is `true`.
    template < bool Cond, typename T = void >
    using enable_if_t = typename enable_if<Cond, T>::type;

    /// @brief Convenience alias for `disable_if<Cond, T>::type`, defaulting \p T to `void`.
    /// @tparam Cond Compile-time boolean gate.
    /// @tparam T Type exposed when \p Cond is `false`.
    template < bool Cond, typename T = void >
    using disable_if_t = typename disable_if<Cond, T>::type;
} // namespace hsd
