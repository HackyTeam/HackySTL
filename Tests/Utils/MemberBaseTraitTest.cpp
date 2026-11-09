#include <cassert>
#include <type_traits>

#include <Base/TypeTraits.hpp>

// hsd::is_member_function_pointer and hsd::is_base_of, checked against explicit expectations
// and against the standard traits, so that a disagreement is visible. Each expectation is in an
// extra pair of parentheses, so that the commas of template arguments are not split by assert().
namespace
{
    struct base {};
    struct derived : base {};
    struct private_derived : private base {};
    struct unrelated {};
    union plain_union { int value; };

    struct holder
    {
        int data;
        void plain();
        void constant() const;
        void volatile_method() volatile;
        void lvalue() &;
        void rvalue() &&;
        void throwing() noexcept;
        void variadic(int, ...);
        static void free_function();
    };

    template <typename T>
    bool member_function_is(bool expected)
    {
        return hsd::is_member_function_pointer<T>::value == expected
            && std::is_member_function_pointer_v<T> == expected;
    }

    template <typename Base, typename Derived>
    bool base_is(bool expected)
    {
        return hsd::is_base_of<Base, Derived>::value == expected
            && std::is_base_of_v<Base, Derived> == expected;
    }
}

int main()
{
    // Pointers to member functions, with each qualifier that changes the type.
    assert((member_function_is<void (holder::*)()>(true)));
    assert((member_function_is<void (holder::*)() const>(true)));
    assert((member_function_is<void (holder::*)() volatile>(true)));
    assert((member_function_is<void (holder::*)() &>(true)));
    assert((member_function_is<void (holder::*)() &&>(true)));
    assert((member_function_is<void (holder::*)() noexcept>(true)));
    assert((member_function_is<void (holder::*)(int, ...)>(true)));
    assert((member_function_is<void (holder::*)(int, ...) const & noexcept>(true)));

    // A cv-qualified pointer is still a pointer to a member function.
    assert((member_function_is<void (holder::* const)()>(true)));
    assert((member_function_is<void (holder::* volatile)() const>(true)));

    // Pointers to data members, plain function pointers and other types are not.
    assert((member_function_is<int holder::*>(false)));
    assert((member_function_is<int holder::* const>(false)));
    assert((member_function_is<void (*)()>(false)));
    assert((member_function_is<void (*)(int, ...)>(false)));
    assert((member_function_is<decltype(&holder::free_function)>(false)));
    assert((member_function_is<int>(false)));
    assert((member_function_is<int*>(false)));
    assert((member_function_is<void>(false)));

    static_assert(hsd::is_member_function_pointer<void (holder::*)() const &>::value);
    static_assert(!hsd::is_member_function_pointer<int holder::*>::value);

    // A class is its own base, and a derived class has its base whatever the inheritance access.
    assert((base_is<base, base>(true)));
    assert((base_is<base, derived>(true)));
    assert((base_is<base, private_derived>(true)));
    assert((base_is<const base, derived>(true)));
    assert((base_is<base, const derived>(true)));

    // The other way round, or an unrelated class, is not a base.
    assert((base_is<derived, base>(false)));
    assert((base_is<base, unrelated>(false)));

    // Only classes have bases: scalars, unions, references and void never do.
    assert((base_is<int, int>(false)));
    assert((base_is<int, derived>(false)));
    assert((base_is<plain_union, plain_union>(false)));
    assert((base_is<base&, derived>(false)));
    assert((base_is<void, void>(false)));

    static_assert(hsd::is_base_of<base, derived>::value);
    static_assert(!hsd::is_base_of<derived, base>::value);
    return 0;
}
