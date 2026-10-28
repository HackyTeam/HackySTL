// Title: Spin lock and CAS maximum
// Level: Advanced
// Shows: a spin lock built on atomic_flag, and a lock-free maximum with a compare-exchange loop.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Atomic.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

// A minimal spin lock: acquire spins until test_and_set sees the flag clear.
class spin_lock
{
private:
    hsd::atomic_flag _flag{};

public:
    void lock()
    {
        while (_flag.test_and_set(hsd::memory_order_acquire))
        {
            hsd::this_thread::yield();
        }
    }

    void unlock()
    {
        _flag.clear(hsd::memory_order_release);
    }
};

// Raises target to value when value is bigger. If another thread changes
// target between the load and the swap, compare_exchange_strong refreshes
// current and the loop tries again.
void update_max(hsd::atomic_i32& target, hsd::i32 value)
{
    hsd::i32 current = target.load();

    while (value > current)
    {
        if (target.compare_exchange_strong(current, value))
        {
            return;
        }
    }
}

int main()
{
    constexpr hsd::i32 worker_count = 4;
    constexpr hsd::i32 per_worker = 1000;

    spin_lock lock;
    hsd::i64 protected_sum = 0;
    hsd::atomic_i32 maximum{0};
    hsd::vector<hsd::thread> workers;

    for (hsd::i32 w = 0; w < worker_count; ++w)
    {
        workers.emplace_back([&, w]
        {
            for (hsd::i32 i = 0; i < per_worker; ++i)
            {
                // A fixed scrambling of the index gives every thread varied values.
                hsd::i32 value = (i * 37 + w * 101) % 1009;

                lock.lock();
                protected_sum += value;
                lock.unlock();

                update_max(maximum, value);
            }
        });
    }

    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    // The same numbers are recomputed on one thread to check the result.
    hsd::i64 expected_sum = 0;
    hsd::i32 expected_max = 0;

    for (hsd::i32 w = 0; w < worker_count; ++w)
    {
        for (hsd::i32 i = 0; i < per_worker; ++i)
        {
            hsd::i32 value = (i * 37 + w * 101) % 1009;
            expected_sum += value;

            if (value > expected_max)
            {
                expected_max = value;
            }
        }
    }

    hsd::println("sum: {}"_fmt, protected_sum);
    hsd::println("max: {}"_fmt, maximum.load());
    hsd::println("sum matches: {}"_fmt, protected_sum == expected_sum ? "true" : "false");
    hsd::println("max matches: {}"_fmt, maximum.load() == expected_max ? "true" : "false");
    return 0;
}
