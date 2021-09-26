#pragma once

#include "Utility.hpp"

namespace hsd
{
    template <typename T>
    concept IsIntegral = is_integral<T>::value;
    template <typename T>
    concept IsFloat = is_floating_point<T>::value;
    template <typename T>
    concept IsSigned = is_signed<T>::value;
    template <typename T>
    concept IsUnsigned = is_unsigned<T>::value;
    template <typename T>
    concept IsNatural = IsIntegral<T> && IsUnsigned<T>;
    template <typename T>
    concept IsNumber = IsIntegral<T> || IsFloat<T>;
    template <typename T>
    concept IsObject = is_object<T>::value;
    template <typename T>
    concept IsVoid = is_void<T>::value;
    template <typename T>
    concept IsReference = is_reference<T>::value;
    template <typename T>
    concept IsRvalueRef = is_rvalue_reference<T>::value;
    template <typename T>
    concept IsLvalueRef = is_lvalue_reference<T>::value;
    template <typename T>
    concept IsConst = is_const<T>::value;
    template <typename T>
    concept IsArray = is_array<T>::value;
    template <typename T>
    concept IsFunction = is_function<T>::value;
    template <typename T, typename U>
    concept IsSame = is_same<T, U>::value;
    template <typename T>
    concept IsPointer = is_pointer<T>::value;
    
    template <typename T>
    concept NumericType = IsNumber<T>;
    template <typename T>
    concept IntegralType = IsIntegral<T>;
    template <typename T>
    concept FloatingPointType = IsFloat<T>;
    template <typename T>
    concept SignedType = IsSigned<T>;
    template <typename T>
    concept UnsignedType = IsUnsigned<T>;
    template <typename T>
    concept NaturalType = IsNatural<T>;
    template <typename T>
    concept ObjectType = IsObject<T>;
    template <typename T>
    concept VoidType = IsVoid<T>;
    template <typename T>
    concept ReferenceType = IsReference<T>;
    template <typename T>
    concept RvalueReferenceType = IsRvalueRef<T>;
    template <typename T>
    concept LvalueReferenceType = IsLvalueRef<T>;
    template <typename T>
    concept ConstType = IsConst<T>;
    template <typename T>
    concept ArrayType = IsArray<T>;
    template <typename T>
    concept FunctionType = IsFunction<T>;
    template <typename T>
    concept PointerType = IsPointer<T>;

    template < typename Func, typename... Args >
    concept Invocable = requires(Func func, Args... args) 
    {
        func(forward<Args>(args)...);
    };

    template < typename RetType, typename Func, typename... Args >
    concept InvocableRet = ( 
        requires(Func func, Args... args){ {func(forward<Args>(args)...)} -> IsSame<RetType>; } ||
        requires(Func func, Args... args){ {func(forward<Args>(args)...).unwrap()} -> IsSame<RetType>; }
    );

    template < typename Func, typename... Args >
    concept UnwrapInvocable = requires(Func func, Args... args)
    {
        func(forward<Args>(args)...).unwrap();
    };

    template < typename T, typename... Args >
    concept Constructible = requires(Args... args)
    {
        { T{forward<Args>(args)...} };
    };

    template <typename T>
    concept DefaultConstructible = Constructible<T>;

    template <typename T>
    concept CopyConstructible = Constructible<T, const T&> || Constructible<T, T&>;

    template <typename T>
    concept MoveConstructible = Constructible<T, T&&>;

    template <typename T>
    concept CopyAssignable = requires(T a, T b)
    {
        a = b;
    };

    template <typename T>
    concept MoveAssignable = requires(T a, T b)
    {
        a = move(b);
    };

    template <typename From, typename To>
    concept Convertible = requires(From from)
    {
        static_cast<To>(from);
    };

    template <typename T, typename... Args>
    concept LiteralConstructible = requires(T t, Args... args)
    {
        {literal_construct(t, forward<Args>(args)...)};
    };

    template <typename T>
    concept BasicIterable = (
        requires(T value) { *value; }  &&
        requires(T lhs, T rhs) { {lhs != rhs} -> IsSame<bool>; } &&
        requires(T lhs, T rhs) { {lhs == rhs} -> IsSame<bool>; }
    );

    template <typename T>
    concept ForwardIterable = (
        requires(T value) { value++; } && BasicIterable<T>
    );

    template <typename T>
    concept ContiguousForwardIterable = (
        requires(T first, T second) { {first - second} -> IsSame<isize>; } &&
        requires(T value) { value++; } && BasicIterable<T>
    );

    template <typename T>
    concept ReverseIterable = (
        requires(T value) { value--; } && BasicIterable<T>
    );

    template <typename T>
    concept BidirectionalIterable = (
        ReverseIterable<T> &&
        ForwardIterable<T>
    );

    template <typename Container>
    concept IsForwardContainer = (
        requires(Container value) 
        { 
            {value.begin()} -> IsSame<decltype(value.end())>;
        } && requires(Container value) 
        {
            ForwardIterable<decltype(value.end())>;
        } && requires(Container value) 
        {
            {value.size()} -> IsSame<usize>;
        }
    );

    template <typename Container>
    concept IsReverseContainer = (
        requires(Container value) 
        { 
            {value.rbegin()} -> IsSame<decltype(value.rend())>;
        } && requires(Container value) 
        {
            ReverseIterable<decltype(value.rend())>;
        } && requires(Container value) 
        {
            {value.size()} -> IsSame<usize>;
        }
    );

    template <typename T>
    concept IsContainer = IsForwardContainer<T> || IsReverseContainer<T>;

    template <typename T>
    concept IsBidirectionalContainer = IsForwardContainer<T> && IsReverseContainer<T>;
} // namespace hsd
