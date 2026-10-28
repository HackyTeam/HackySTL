#pragma once

/// @file Concepts.hpp
/// @brief Concepts describing HackySTL's supported type and iterator contracts.
/// @details These constraints are shared by containers, wrappers, formatting,
/// threading, and algorithms to keep overload participation explicit.

#include "Types.hpp"
#include "TypeTraits.hpp"
#include "Utility.hpp"

namespace hsd
{
    /// @brief Matches types for which `is_integral` holds.
    /// @tparam T Type tested.
    template <typename T>
    concept IsIntegral = is_integral<T>::value;

    /// @brief Matches types for which `is_floating_point` holds.
    /// @tparam T Type tested.
    template <typename T>
    concept IsFloat = is_floating_point<T>::value;

    /// @brief Matches types for which `is_signed` holds.
    /// @tparam T Type tested.
    template <typename T>
    concept IsSigned = is_signed<T>::value;

    /// @brief Matches types for which `is_unsigned` holds.
    /// @tparam T Type tested.
    template <typename T>
    concept IsUnsigned = is_unsigned<T>::value;

    /// @brief Matches unsigned integral types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsNatural = IsIntegral<T> && IsUnsigned<T>;

    /// @brief Matches integral or floating-point types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsNumber = IsIntegral<T> || IsFloat<T>;

    /// @brief Matches types for which `is_object` holds.
    /// @tparam T Type tested.
    template <typename T>
    concept IsObject = is_object<T>::value;

    /// @brief Matches `void`.
    /// @tparam T Type tested.
    template <typename T>
    concept IsVoid = is_void<T>::value;

    /// @brief Matches lvalue and rvalue reference types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsReference = is_reference<T>::value;

    /// @brief Matches rvalue reference types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsRvalueRef = is_rvalue_reference<T>::value;

    /// @brief Matches lvalue reference types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsLvalueRef = is_lvalue_reference<T>::value;

    /// @brief Matches const-qualified types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsConst = is_const<T>::value;

    /// @brief Matches array types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsArray = is_array<T>::value;

    /// @brief Matches function types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsFunction = is_function<T>::value;

    /// @brief Matches when `T` and `U` are the same type.
    /// @tparam T First type.
    /// @tparam U Second type.
    template <typename T, typename U>
    concept IsSame = is_same<T, U>::value;

    /// @brief Matches pointer types.
    /// @tparam T Type tested.
    template <typename T>
    concept IsPointer = is_pointer<T>::value;

    /// @brief Matches class types, meaning structs and classes (not unions or enums).
    /// @tparam T Type tested.
    template <typename T>
    concept IsClass = is_class<T>::value;

    /// @brief Matches types whose destructor does nothing.
    /// @tparam T Type tested.
    template <typename T>
    concept IsTriviallyDestructible = is_trivially_destructible<T>::value;

    /// @brief Matches the character types the library supports.
    /// @tparam T Type tested.
    /// @details The set is `char`, `uchar`, `char16`, `char32`, `wchar` and `schar`.
    template <typename T>
    concept IsChar = (
        is_same<T, char>::value   ||
        is_same<T, uchar>::value  ||
        is_same<T, char16>::value ||
        is_same<T, char32>::value ||
        is_same<T, wchar>::value  ||
        is_same<T, schar>::value
    );

    /// @brief Alias of `IsNumber`.
    /// @tparam T Type tested.
    template <typename T>
    concept NumericType = IsNumber<T>;

    /// @brief Alias of `IsIntegral`.
    /// @tparam T Type tested.
    template <typename T>
    concept IntegralType = IsIntegral<T>;

    /// @brief Alias of `IsFloat`.
    /// @tparam T Type tested.
    template <typename T>
    concept FloatingPointType = IsFloat<T>;

    /// @brief Alias of `IsSigned`.
    /// @tparam T Type tested.
    template <typename T>
    concept SignedType = IsSigned<T>;

    /// @brief Alias of `IsUnsigned`.
    /// @tparam T Type tested.
    template <typename T>
    concept UnsignedType = IsUnsigned<T>;

    /// @brief Alias of `IsNatural`.
    /// @tparam T Type tested.
    template <typename T>
    concept NaturalType = IsNatural<T>;

    /// @brief Alias of `IsObject`.
    /// @tparam T Type tested.
    template <typename T>
    concept ObjectType = IsObject<T>;

    /// @brief Alias of `IsVoid`.
    /// @tparam T Type tested.
    template <typename T>
    concept VoidType = IsVoid<T>;

    /// @brief Alias of `IsReference`.
    /// @tparam T Type tested.
    template <typename T>
    concept ReferenceType = IsReference<T>;

    /// @brief Alias of `IsRvalueRef`.
    /// @tparam T Type tested.
    template <typename T>
    concept RvalueReferenceType = IsRvalueRef<T>;

    /// @brief Alias of `IsLvalueRef`.
    /// @tparam T Type tested.
    template <typename T>
    concept LvalueReferenceType = IsLvalueRef<T>;

    /// @brief Alias of `IsConst`.
    /// @tparam T Type tested.
    template <typename T>
    concept ConstType = IsConst<T>;

    /// @brief Alias of `IsArray`.
    /// @tparam T Type tested.
    template <typename T>
    concept ArrayType = IsArray<T>;

    /// @brief Alias of `IsFunction`.
    /// @tparam T Type tested.
    template <typename T>
    concept FunctionType = IsFunction<T>;

    /// @brief Alias of `IsPointer`.
    /// @tparam T Type tested.
    template <typename T>
    concept PointerType = IsPointer<T>;

    /// @brief Alias of `IsClass`.
    /// @tparam T Type tested.
    template <typename T>
    concept ClassType = IsClass<T>;

    /// @brief Alias of `IsTriviallyDestructible`.
    /// @tparam T Type tested.
    template <typename T>
    concept TriviallyDestructibleType = IsTriviallyDestructible<T>;

    /// @brief Alias of `IsChar`.
    /// @tparam T Type tested.
    template <typename T>
    concept CharacterType = IsChar<T>;

    /// @brief Matches a callable that accepts `Args...` as arguments.
    /// @tparam Func Callable type.
    /// @tparam Args Argument types the call is tested with.
    template < typename Func, typename... Args >
    concept Invocable = requires(Func func, Args... args)
    {
        func(hsd::forward<Args>(args)...);
    };

    /// @brief Matches a callable whose result is `RetType`, directly or after `unwrap()`.
    /// @tparam RetType Required result type.
    /// @tparam Func Callable type.
    /// @tparam Args Argument types the call is tested with.
    template < typename RetType, typename Func, typename... Args >
    concept InvocableRet = (
        requires(Func func, Args... args){ {func(hsd::forward<Args>(args)...)} -> IsSame<RetType>; } ||
        requires(Func func, Args... args){ {func(hsd::forward<Args>(args)...).unwrap()} -> IsSame<RetType>; }
    );

    /// @brief Matches a callable whose result provides `unwrap()`.
    /// @tparam Func Callable type.
    /// @tparam Args Argument types the call is tested with.
    template < typename Func, typename... Args >
    concept UnwrapInvocable = requires(Func func, Args... args)
    {
        func(hsd::forward<Args>(args)...).unwrap();
    };

    /// @brief Matches types that can be list-initialized from `Args...`.
    /// @tparam T Type constructed.
    /// @tparam Args Argument types the construction is tested with.
    template < typename T, typename... Args >
    concept Constructible = requires(Args... args)
    {
        { T{hsd::forward<Args>(args)...} };
    };

    /// @brief Matches types that can be constructed with no arguments.
    /// @tparam T Type constructed.
    template <typename T>
    concept DefaultConstructible = Constructible<T>;

    /// @brief Matches types that can be constructed from an lvalue or a const lvalue.
    /// @tparam T Type constructed.
    template <typename T>
    concept CopyConstructible = Constructible<T, const T&> || Constructible<T, T&>;

    /// @brief Matches types that can be constructed from an rvalue.
    /// @tparam T Type constructed.
    template <typename T>
    concept MoveConstructible = Constructible<T, T&&>;

    /// @brief Matches types whose objects can be assigned from another object of the same type.
    /// @tparam T Type tested.
    template <typename T>
    concept CopyAssignable = requires(T a, T b)
    {
        a = b;
    };

    /// @brief Matches types whose objects can be assigned from an rvalue of the same type.
    /// @tparam T Type tested.
    template <typename T>
    concept MoveAssignable = requires(T a, T b)
    {
        a = hsd::move(b);
    };

    /// @brief Matches when a `From` value can be explicitly converted to `To`.
    /// @tparam From Source type.
    /// @tparam To Target type.
    template <typename From, typename To>
    concept Convertible = requires(From from)
    {
        static_cast<To>(from);
    };

    /// @brief Matches a reference type `T` that can be converted to `U` and assigned from `U`.
    /// @tparam T Reference type tested.
    /// @tparam U Type the reference is converted to and assigned from.
    template <typename T, typename U>
    concept RefConstructible = (
        IsReference<T> && Convertible<T, U> && requires(T t, U u)
        {
            t = u;
        }
    );

    /// @brief Matches types that can be assigned a compile-time value built from `Args...`.
    /// @tparam T Destination type.
    /// @tparam Args Argument types the construction is tested with.
    /// @details Checks the assignment `dest = T{args...}` that `literal_construct`
    /// performs. Calling `literal_construct` here would not check that assignment,
    /// because the body of a called function is not instantiated while a
    /// `requires` clause is checked.
    template <typename T, typename... Args>
    concept LiteralConstructible = requires(T t, Args... args)
    {
        t = T{hsd::forward<Args>(args)...};
    };

    /// @brief Matches types that can be dereferenced and compared for equality and inequality.
    /// @tparam T Iterator type tested.
    template <typename T>
    concept BasicIterable = (
        requires(T value) { *value; }  &&
        requires(T lhs, T rhs) { {lhs != rhs} -> IsSame<bool>; } &&
        requires(T lhs, T rhs) { {lhs == rhs} -> IsSame<bool>; }
    );

    /// @brief Matches basic iterators that can be incremented.
    /// @tparam T Iterator type tested.
    template <typename T>
    concept ForwardIterable = (
        requires(T value) { value++; } && BasicIterable<T>
    );

    /// @brief Matches forward iterators whose distance is an `isize`.
    /// @tparam T Iterator type tested.
    template <typename T>
    concept ContiguousForwardIterable = (
        requires(T first, T second) { {first - second} -> IsSame<isize>; } &&
        requires(T value) { value++; } && BasicIterable<T>
    );

    /// @brief Matches basic iterators that can be decremented.
    /// @tparam T Iterator type tested.
    template <typename T>
    concept ReverseIterable = (
        requires(T value) { value--; } && BasicIterable<T>
    );

    /// @brief Matches iterators that can move in both directions.
    /// @tparam T Iterator type tested.
    template <typename T>
    concept BidirectionalIterable = (
        ReverseIterable<T> &&
        ForwardIterable<T>
    );

    /// @brief Matches containers whose `begin()` and `end()` yield forward iterators.
    /// @tparam Container Container type tested.
    template <typename Container>
    concept IsForwardContainer = (
        requires(Container value)
        {
            {value.begin()} -> IsSame<decltype(value.end())>;
        } && requires(Container value)
        {
            requires ForwardIterable<decltype(value.end())>;
        }
    );

    /// @brief Matches containers whose `rbegin()` and `rend()` yield reverse iterators.
    /// @tparam Container Container type tested.
    template <typename Container>
    concept IsReverseContainer = (
        requires(Container value)
        {
            {value.rbegin()} -> IsSame<decltype(value.rend())>;
        } && requires(Container value)
        {
            requires ReverseIterable<decltype(value.rend())>;
        }
    );

    /// @brief Matches a forward or a reverse container.
    /// @tparam T Container type tested.
    template <typename T>
    concept IsContainer = IsForwardContainer<T> || IsReverseContainer<T>;

    /// @brief Matches a container that is both a forward and a reverse container.
    /// @tparam T Container type tested.
    template <typename T>
    concept IsBidirectionalContainer = IsForwardContainer<T> && IsReverseContainer<T>;
} // namespace hsd
