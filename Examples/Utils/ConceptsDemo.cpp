// Title: Concepts and overloads
// Level: Intermediate
// Shows: the ready-made concepts in Concepts.hpp used to constrain templates and pick overloads.

#include <Base/Concepts.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

struct point
{
    hsd::i32 x;
    hsd::i32 y;
};

struct move_only
{
    move_only() = default;
    move_only(const move_only&) = delete;
    move_only(move_only&&) = default;
};

// Each overload accepts one family of types, so the call site picks the right
// one without any enable_if.
template <hsd::IntegralType T>
const char* kind_of(const T&)
{
    return hsd::IsSigned<T> ? "signed integer" : "unsigned integer";
}

template <hsd::FloatingPointType T>
const char* kind_of(const T&)
{
    return "floating point";
}

template <hsd::PointerType T>
const char* kind_of(const T&)
{
    return "pointer";
}

template <typename T>
requires (!hsd::NumericType<T> && !hsd::PointerType<T>)
const char* kind_of(const T&)
{
    return "other";
}

// A constrained function only exists for types that fit its job.
template <hsd::NumericType T>
T twice(T value)
{
    return value + value;
}

template <typename Func, typename... Args>
requires (hsd::Invocable<Func, Args...>)
auto call_checked(Func func, Args... args)
{
    return func(args...);
}

const char* yes_no(bool value)
{
    return value ? "true" : "false";
}

int main()
{
    hsd::println("42: {}"_fmt, kind_of(42));
    hsd::println("42u: {}"_fmt, kind_of(42u));
    hsd::println("2.5: {}"_fmt, kind_of(2.5));
    const char* text = "abc";
    hsd::println("text: {}"_fmt, kind_of(text));
    hsd::println("point: {}"_fmt, kind_of(point{1, 2}));

    hsd::println("twice(21)={}"_fmt, twice(21));
    hsd::println("twice(1.25)={prec=2}"_fmt, twice(1.25));
    hsd::println("call_checked={}"_fmt, call_checked([](int a, int b) { return a * b; }, 6, 7));

    // Concepts are plain booleans in a constant expression, so they can be printed.
    hsd::println("point copyable: {}"_fmt, yes_no(hsd::CopyConstructible<point>));
    hsd::println("move_only copyable: {}"_fmt, yes_no(hsd::CopyConstructible<move_only>));
    hsd::println("move_only movable: {}"_fmt, yes_no(hsd::MoveConstructible<move_only>));
    hsd::println("int to double: {}"_fmt, yes_no(hsd::Convertible<int, double>));
    hsd::println("int to pointer: {}"_fmt, yes_no(hsd::Convertible<int, int*>));
    hsd::println("vector is container: {}"_fmt, yes_no(hsd::IsContainer<hsd::vector<int>>));
    hsd::println("int is container: {}"_fmt, yes_no(hsd::IsContainer<int>));

    return 0;
}
