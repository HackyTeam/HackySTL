// Title: Any basics
// Level: Basic
// Shows: storing a value in hsd::any, checking its type, casting, copying and resetting.

#include <Serialize/Io.hpp>
#include <Wrappers/Any.hpp>

using namespace hsd::format_literals;

static const char* yes_no(bool value)
{
    return value ? "yes" : "no";
}

int main()
{
    // An empty any holds nothing, and every type check fails.
    hsd::any value;
    hsd::println("empty: has_value={} int={}"_fmt,
        yes_no(value.has_value()), yes_no(value.holds_type<hsd::i32>()));

    // Assigning a value stores it together with its exact type.
    value = 42;
    hsd::println("stored: has_value={} int={} f64={}"_fmt,
        yes_no(value.has_value()), yes_no(value.holds_type<hsd::i32>()),
        yes_no(value.holds_type<hsd::f64>()));

    // cast_if returns a pointer, which is null when the type is wrong.
    if (auto* number = value.cast_if<hsd::i32>())
    {
        hsd::println("cast_if<i32>: {}"_fmt, *number);
    }
    hsd::println("cast_if<f64> is null: {}"_fmt, yes_no(value.cast_if<hsd::f64>() == nullptr));

    // cast_to returns a result, so a wrong type is an error value, not a crash.
    auto as_int = value.cast_to<hsd::i32>();
    auto as_double = value.cast_to<hsd::f64>();
    hsd::println("cast_to<i32> ok={} cast_to<f64> ok={}"_fmt,
        yes_no(as_int.is_ok()), yes_no(as_double.is_ok()));

    // Copying an any copies the stored object, so the two are independent.
    hsd::any copy{value};
    *copy.cast_if<hsd::i32>() = 7;
    hsd::println("copies differ: {} vs {}"_fmt, *value.cast_if<hsd::i32>(), *copy.cast_if<hsd::i32>());

    // Assigning another type replaces the old value.
    value = 2.5;
    hsd::println("now f64={} value={prec=1}"_fmt,
        yes_no(value.holds_type<hsd::f64>()), *value.cast_if<hsd::f64>());

    value.reset();
    hsd::println("after reset: has_value={}"_fmt, yes_no(value.has_value()));
    return 0;
}
