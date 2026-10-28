// Title: Comparing lock types
// Level: Intermediate
// Shows: hsd::mutex, hsd::futex and hsd::spin behind the same unique_lock code.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Lock.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

// The same program runs with any type that has lock() and unlock().
template <typename Lock>
hsd::i32 count_with(hsd::i32 thread_count, hsd::i32 rounds)
{
    Lock guard_lock;
    hsd::i32 counter = 0;
    hsd::vector<hsd::thread> workers;

    for (hsd::i32 w = 0; w < thread_count; ++w)
    {
        workers.emplace_back([&guard_lock, &counter, rounds]
        {
            for (hsd::i32 i = 0; i < rounds; ++i)
            {
                hsd::unique_lock<Lock> lock{guard_lock};
                ++counter;
            }
        });
    }

    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    return counter;
}

int main()
{
    // The mutex sleeps in the kernel while it waits.
    hsd::println("mutex: {}"_fmt, count_with<hsd::mutex>(4, 2000));

    // The futex only enters the kernel when there is contention.
    hsd::println("futex: {}"_fmt, count_with<hsd::futex>(4, 2000));

    // The spin lock busy-waits, which suits very short critical sections.
    hsd::println("spin: {}"_fmt, count_with<hsd::spin>(4, 2000));

    // Only mutex and spin offer try_lock.
    hsd::mutex mutex;
    hsd::spin spin;
    hsd::println("mutex try_lock: {}"_fmt, mutex.try_lock() ? "true" : "false");
    hsd::println("mutex try_lock again: {}"_fmt, mutex.try_lock() ? "true" : "false");
    hsd::println("spin try_lock: {}"_fmt, spin.try_lock() ? "true" : "false");
    hsd::println("spin try_lock again: {}"_fmt, spin.try_lock() ? "true" : "false");
    mutex.unlock();
    spin.unlock();

    // dont_lock builds the guard without locking, and lock() takes it later.
    hsd::unique_lock<hsd::mutex> deferred{hsd::dont_lock, mutex};
    hsd::println("deferred owns: {}"_fmt, deferred.is_locked() ? "true" : "false");
    deferred.lock().unwrap();
    hsd::println("deferred protects mutex: {}"_fmt, deferred.protects(&mutex) ? "true" : "false");

    return 0;
}
