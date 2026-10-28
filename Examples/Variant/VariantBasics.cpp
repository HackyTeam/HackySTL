// Title: Variant basics
// Level: Basic
// Shows: hsd::variant holding one of several types, with index, holds, get and get_if.

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Variant.hpp>

using namespace hsd::format_literals;

static const char* yes_no(bool value)
{
    return value ? "yes" : "no";
}

int main()
{
    // A variant always holds exactly one of its alternatives. A default
    // constructed one holds the first alternative, value-initialized.
    hsd::variant<hsd::i32, hsd::f64, hsd::string> value;
    hsd::println("default: index {} holds i32 {} value {}"_fmt,
        value.index(), yes_no(value.holds<hsd::i32>()), value.get<hsd::i32>().unwrap().get());

    value = 42;
    hsd::println("int: index {}"_fmt, value.index());

    value = 2.5;
    hsd::println("double: index {} holds f64 {}"_fmt, value.index(), yes_no(value.holds<hsd::f64>()));

    value = hsd::string{"text"};
    hsd::println("string: index {} holds string {}"_fmt, value.index(), yes_no(value.holds<hsd::string>()));

    // get<T>() returns a result: asking for the wrong type is an error, not undefined behavior.
    auto wrong = value.get<hsd::i32>();
    auto right = value.get<hsd::string>();
    hsd::println("get<i32> ok: {}"_fmt, yes_no(wrong.is_ok()));
    hsd::println("get<string> ok: {}"_fmt, yes_no(right.is_ok()));

    // get_if<T>() gives a pointer, which is null for a different alternative.
    if (auto* text = value.get_if<hsd::string>())
    {
        hsd::println("get_if<string>: {}"_fmt, text->c_str());
    }
    hsd::println("get_if<i32> is null: {}"_fmt, yes_no(value.get_if<hsd::i32>() == nullptr));

    // A reference from get_if can modify the stored value in place.
    value = 10;
    *value.get_if<hsd::i32>() += 5;
    hsd::println("after += 5: {}"_fmt, *value.get_if<hsd::i32>());

    // Copies compare equal when they hold the same alternative and value.
    hsd::variant<hsd::i32, hsd::f64, hsd::string> copy{value};
    hsd::println("copy equals: {}"_fmt, yes_no(copy == value));
    copy = 99;
    hsd::println("changed copy equals: {}"_fmt, yes_no(copy == value));
    // A variant built from an lvalue stores a copy of it.
    double measured = 2.5;
    hsd::variant<double, hsd::string> from_lvalue{measured};
    hsd::println("lvalue stored as index {}"_fmt, from_lvalue.index());
    return 0;
}
