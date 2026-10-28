// Title: Waiting with a timeout
// Level: Advanced
// Shows: wait_for and wait_until with and without a predicate, and cv_status.

#include <Serialize/Io.hpp>
#include <Threading/ConditionVariable.hpp>
#include <Threading/Lock.hpp>
#include <Threading/MonotonicClock.hpp>
#include <Threading/Thread.hpp>
#include <Threading/Time.hpp>

using namespace hsd::format_literals;
using namespace hsd::precise_clock_literals;

int main()
{
    hsd::mutex mutex;
    hsd::condition_variable cond_var;
    bool ready = false;

    hsd::unique_lock<hsd::mutex> lock{mutex};

    // Nobody notifies, so the wait ends because the time ran out.
    // Timings vary between runs, so only comparisons are printed.
    hsd::u64 start = hsd::monotonic_clock::now_ns();
    hsd::cv_status status = cond_var.wait_for(lock, 50_ms);
    hsd::u64 elapsed = hsd::monotonic_clock::now_ns() - start;
    hsd::println("timed out: {}"_fmt, status == hsd::cv_status::timeout ? "true" : "false");
    hsd::println("waited at least 40 ms: {}"_fmt, elapsed >= 40'000'000 ? "true" : "false");

    // The predicate form keeps waiting until the deadline and then
    // returns the value of the predicate, which is still false.
    bool satisfied = cond_var.wait_for(lock, 30_ms, [&] { return ready; });
    hsd::println("predicate satisfied: {}"_fmt, satisfied ? "true" : "false");

    // wait_until takes an absolute deadline on the wall clock.
    hsd::precise_clock deadline = hsd::precise_clock::now() + 30_ms;
    hsd::cv_status until_status = cond_var.wait_until(lock, deadline);
    hsd::println("wait_until timed out: {}"_fmt, until_status == hsd::cv_status::timeout ? "true" : "false");

    // A notifier that arrives in time wakes the waiter long before the deadline.
    hsd::thread notifier{[&]
    {
        hsd::this_thread::sleep_for(0.03f).unwrap();
        {
            hsd::unique_lock<hsd::mutex> inner{mutex};
            ready = true;
        }

        cond_var.notify_all();
    }};

    start = hsd::monotonic_clock::now_ns();
    bool woke = cond_var.wait_for(lock, 5_s, [&] { return ready; });
    elapsed = hsd::monotonic_clock::now_ns() - start;
    hsd::println("woken by notify: {}"_fmt, woke ? "true" : "false");
    hsd::println("finished before the deadline: {}"_fmt, elapsed < 4'000'000'000 ? "true" : "false");

    lock.unlock().unwrap();
    notifier.join().unwrap();
    return 0;
}
