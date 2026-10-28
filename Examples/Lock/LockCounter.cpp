// Title: Locked counter
// Level: Basic
// Shows: protecting a shared counter with hsd::mutex and hsd::unique_lock.

#include <Serialize/Io.hpp>
#include <Threading/Lock.hpp>
#include <Threading/Thread.hpp>
#include <Containers/Vector.hpp>

using namespace hsd::format_literals;

int main()
{
    hsd::mutex counter_mutex;
    hsd::i32 counter = 0;

    hsd::vector<hsd::thread> workers;

    for (hsd::i32 w = 0; w < 4; ++w)
    {
        workers.emplace_back([&counter_mutex, &counter]
        {
            for (hsd::i32 i = 0; i < 1000; ++i)
            {
                // The lock is taken here and released when it leaves the scope.
                hsd::unique_lock<hsd::mutex> lock{counter_mutex};
                ++counter;
            }
        });
    }

    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    hsd::println("counter: {}"_fmt, counter);

    // try_lock fails instead of waiting when the mutex is already taken.
    hsd::println("first try_lock: {}"_fmt, counter_mutex.try_lock() ? "true" : "false");
    hsd::println("second try_lock: {}"_fmt, counter_mutex.try_lock() ? "true" : "false");
    counter_mutex.unlock();

    // A unique_lock can be released and taken again by hand.
    hsd::unique_lock<hsd::mutex> lock{counter_mutex};
    hsd::println("owns lock: {}"_fmt, lock.is_locked() ? "true" : "false");
    lock.unlock().unwrap();
    hsd::println("owns lock after unlock: {}"_fmt, lock.is_locked() ? "true" : "false");

    // Unlocking twice is reported through the result instead of being undefined.
    hsd::println("second unlock ok: {}"_fmt, lock.unlock().is_ok() ? "true" : "false");

    return 0;
}
