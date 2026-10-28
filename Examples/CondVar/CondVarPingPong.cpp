// Title: Ping pong
// Level: Basic
// Shows: two threads taking turns with a condition_variable and a predicate.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/ConditionVariable.hpp>
#include <Threading/Lock.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

int main()
{
    hsd::mutex mutex;
    hsd::condition_variable cond_var;
    bool ping_turn = true;

    // Each thread records what it did, instead of printing from the worker.
    hsd::vector<hsd::i32> log;
    constexpr hsd::i32 rounds = 3;

    hsd::thread ping{[&]
    {
        for (hsd::i32 i = 0; i < rounds; ++i)
        {
            hsd::unique_lock<hsd::mutex> lock{mutex};

            // wait() releases the mutex while it sleeps and re-checks the
            // predicate every time it wakes up.
            cond_var.wait(lock, [&] { return ping_turn; });
            log.push_back(1);
            ping_turn = false;
            lock.unlock().unwrap();
            cond_var.notify_all();
        }
    }};

    hsd::thread pong{[&]
    {
        for (hsd::i32 i = 0; i < rounds; ++i)
        {
            hsd::unique_lock<hsd::mutex> lock{mutex};
            cond_var.wait(lock, [&] { return !ping_turn; });
            log.push_back(2);
            ping_turn = true;
            lock.unlock().unwrap();
            cond_var.notify_all();
        }
    }};

    ping.join().unwrap();
    pong.join().unwrap();

    for (hsd::usize i = 0; i < log.size(); ++i)
    {
        hsd::println("{}"_fmt, log[i] == 1 ? "ping" : "pong");
    }

    return 0;
}
