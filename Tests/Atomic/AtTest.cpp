#include <cassert>
#include <atomic>
#include <Threading/Thread.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Atomic.hpp>
 
hsd::atomic_flag lock{};
hsd::atomic_i32 completed{};
std::atomic<hsd::i32> standard_completed{};
 
void thread_func([[maybe_unused]] hsd::i32 thread_index)
{
    using namespace hsd::format_literals;

    for (hsd::i32 count = 0; count < 100; count++)
    {
        // spin until the lock is acquired
        while (lock.test_and_set(hsd::memory_order_acquire));

        completed.fetch_add(1, hsd::memory_order_relaxed);
        standard_completed.fetch_add(1, std::memory_order_relaxed);
        lock.clear(hsd::memory_order_release);
    }
}
 
int main()
{
    hsd::atomic_i32 counter{5};
    std::atomic<hsd::i32> standard_counter{5};
    assert(counter.load() == 5);
    assert(counter.load() == standard_counter.load());
    const auto exchanged = counter.exchange(8);
    const auto standard_exchanged = standard_counter.exchange(8);
    assert(exchanged == standard_exchanged);
    assert(exchanged == 5);

    hsd::i32 expected = 7;
    hsd::i32 standard_expected = 7;
    assert(!counter.compare_exchange_strong(expected, 9));
    assert(!standard_counter.compare_exchange_strong(standard_expected, 9));
    assert(expected == 8);
    assert(expected == standard_expected);
    assert(counter.load() == 8);
    assert(counter.load() == standard_counter.load());

    expected = 8;
    standard_expected = 8;
    assert(counter.compare_exchange_strong(expected, 10));
    assert(standard_counter.compare_exchange_strong(standard_expected, 10));
    assert(counter.load() == 10);
    assert(counter.load() == standard_counter.load());

    hsd::atomic_u32 flags{0b1010};
    std::atomic<hsd::u32> standard_flags{0b1010};
    const auto and_previous = flags.fetch_and(0b0110);
    const auto standard_and_previous = standard_flags.fetch_and(0b0110);
    assert(and_previous == standard_and_previous);
    assert(and_previous == 0b1010);
    assert(flags.load() == 0b0010);
    assert(flags.load() == standard_flags.load());
    const auto or_previous = flags.fetch_or(0b1000);
    const auto standard_or_previous = standard_flags.fetch_or(0b1000);
    assert(or_previous == standard_or_previous);
    assert(or_previous == 0b0010);
    assert(flags.load() == 0b1010);
    assert(flags.load() == standard_flags.load());
    const auto xor_previous = flags.fetch_xor(0b0011);
    const auto standard_xor_previous = standard_flags.fetch_xor(0b0011);
    assert(xor_previous == standard_xor_previous);
    assert(xor_previous == 0b1010);
    assert(flags.load() == 0b1001);
    assert(flags.load() == standard_flags.load());

    hsd::vector<hsd::thread> vec;

    for (int index = 0; index < 3; index++)
    {
        vec.emplace_back(thread_func, index);
    }

    for (auto& thread_val : vec)
    {
        thread_val.join().unwrap();
    }

    assert(completed.load() == 3 * 100);
    assert(completed.load() == standard_completed.load());
}