// Title: Atomic counters across threads
// Level: Intermediate
// Shows: lock-free counters, a ticket dispenser built on fetch_add, and atomic_bool.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Atomic.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

int main()
{
    constexpr hsd::i32 worker_count = 4;
    constexpr hsd::i32 rounds = 5000;
    constexpr hsd::i32 ticket_total = worker_count * 100;

    hsd::atomic_i32 hits{0};
    hsd::atomic_u64 weighted{0};
    hsd::atomic_i32 next_ticket{0};
    hsd::atomic_bool failed{false};

    // Slot t is written by whoever draws ticket t. Every ticket is
    // unique, so no two threads ever write the same slot.
    hsd::vector<hsd::i32> owner;
    owner.resize(ticket_total);

    hsd::vector<hsd::thread> workers;

    for (hsd::i32 w = 0; w < worker_count; ++w)
    {
        workers.emplace_back([&, w]
        {
            for (hsd::i32 i = 0; i < rounds; ++i)
            {
                hits.fetch_add(1);
                weighted.fetch_add(static_cast<hsd::u64>(w + 1));
            }

            for (hsd::i32 i = 0; i < 100; ++i)
            {
                // fetch_add returns the old value, which is this thread's ticket.
                hsd::i32 ticket = next_ticket.fetch_add(1);

                if (ticket < 0 || ticket >= ticket_total)
                {
                    failed.store(true);
                    return;
                }

                owner[ticket] = w + 1;
            }
        });
    }

    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    hsd::println("hits: {}"_fmt, hits.load());
    hsd::println("weighted: {}"_fmt, weighted.load());
    hsd::println("tickets drawn: {}"_fmt, next_ticket.load());
    hsd::println("ticket out of range: {}"_fmt, failed.load() ? "true" : "false");

    // Each thread drew 100 tickets, so every owner appears exactly 100 times.
    hsd::i32 per_worker[worker_count]{};

    for (hsd::i32 t = 0; t < ticket_total; ++t)
    {
        ++per_worker[owner[t] - 1];
    }

    for (hsd::i32 w = 0; w < worker_count; ++w)
    {
        hsd::println("worker {} drew {} tickets"_fmt, w, per_worker[w]);
    }

    return 0;
}
