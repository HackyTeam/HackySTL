// Title: Atomic operations
// Level: Basic
// Shows: load, store, exchange, compare_exchange_strong, fetch_* and atomic_flag.

#include <Serialize/Io.hpp>
#include <Threading/Atomic.hpp>

using namespace hsd::format_literals;

int main()
{
    hsd::atomic_i32 value{5};
    hsd::println("load: {}"_fmt, value.load());

    value.store(10);
    hsd::println("after store: {}"_fmt, value.load());

    // exchange puts in the new value and returns the old one.
    hsd::i32 old = value.exchange(20);
    hsd::println("exchange returned {}, now {}"_fmt, old, value.load());

    // fetch_add and fetch_sub also return the value from before the change.
    hsd::println("fetch_add returned {}"_fmt, value.fetch_add(5));
    hsd::println("fetch_sub returned {}"_fmt, value.fetch_sub(3));
    hsd::println("value: {}"_fmt, value.load());

    // compare_exchange_strong only stores when the current value is the expected one.
    // On a miss it copies the real value into expected.
    hsd::i32 expected = 99;
    bool swapped = value.compare_exchange_strong(expected, 1);
    hsd::println("swap with wrong guess: {}, expected is now {}"_fmt,
        swapped ? "true" : "false", expected);
    swapped = value.compare_exchange_strong(expected, 1);
    hsd::println("swap with right guess: {}, value is now {}"_fmt,
        swapped ? "true" : "false", value.load());

    // The bitwise forms work on unsigned flags.
    hsd::atomic_u32 flags{0b1010};
    flags.fetch_and(0b0110);
    hsd::println("after and: {}"_fmt, flags.load());
    flags.fetch_or(0b1000);
    hsd::println("after or: {}"_fmt, flags.load());
    flags.fetch_xor(0b0011);
    hsd::println("after xor: {}"_fmt, flags.load());

    // Operators are atomic too.
    hsd::atomic_i32 counter{0};
    ++counter;
    counter += 4;
    hsd::println("counter: {}"_fmt, counter.load());

    // atomic_flag is the smallest atomic. test_and_set returns the previous state.
    hsd::atomic_flag flag{};
    bool first = flag.test_and_set();
    bool second = flag.test_and_set();
    hsd::println("test_and_set: {} then {}"_fmt, first ? "true" : "false", second ? "true" : "false");
    flag.clear();
    hsd::println("after clear: {}"_fmt, flag.test() ? "true" : "false");

    return 0;
}
