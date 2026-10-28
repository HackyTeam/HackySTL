// Title: Function basics
// Level: Basic
// Shows: hsd::function holding a lambda, a function pointer and a stateful lambda, and the empty state.

#include <Serialize/Io.hpp>
#include <Wrappers/Functional.hpp>

using namespace hsd::format_literals;

static hsd::i32 triple(hsd::i32 value)
{
    return value * 3;
}

int main()
{
    // A function<R(Args...)> can hold any callable with that signature.
    hsd::function<hsd::i32(hsd::i32)> op = [](hsd::i32 value) { return value + 10; };

    // Calling it returns a result, because the function might be empty.
    hsd::println("lambda: {}"_fmt, op(5).unwrap());

    // Assigning another callable replaces the target.
    op = triple;
    hsd::println("function pointer: {}"_fmt, op(5).unwrap());

    // A mutable lambda keeps its own state between calls.
    hsd::function<hsd::i32()> counter = [count = 0]() mutable { return ++count; };
    // Call order matters here, so each call is its own statement.
    hsd::i32 first = counter().unwrap();
    hsd::i32 second = counter().unwrap();
    hsd::i32 third = counter().unwrap();
    hsd::println("counter: {} {} {}"_fmt, first, second, third);

    // A copy has its own copy of that state.
    auto counter_copy = counter;
    hsd::i32 from_original = counter().unwrap();
    hsd::i32 from_copy = counter_copy().unwrap();
    hsd::println("original: {} copy: {}"_fmt, from_original, from_copy);

    // A void function returns an option_err; unwrap() just checks that it ran.
    hsd::i32 total = 0;
    hsd::function<void(hsd::i32)> add = [&total](hsd::i32 value) { total += value; };
    add(4).unwrap();
    add(6).unwrap();
    hsd::println("total: {}"_fmt, total);

    // An empty function is falsy, and calling it gives an error instead of crashing.
    hsd::function<hsd::i32()> empty;
    hsd::println("empty is set: {}"_fmt, static_cast<bool>(empty) ? "yes" : "no");
    hsd::println("empty call ok: {}"_fmt, empty().is_ok() ? "yes" : "no");

    // Assigning nullptr clears a function again.
    op = nullptr;
    hsd::println("cleared is set: {}"_fmt, static_cast<bool>(op) ? "yes" : "no");
    return 0;
}
