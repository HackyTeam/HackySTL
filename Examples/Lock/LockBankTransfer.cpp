// Title: Bank transfers
// Level: Advanced
// Shows: one mutex per account, deferred locking, and lock ordering to avoid deadlock.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Lock.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

struct account
{
    hsd::mutex mutex{};
    hsd::i64 balance = 0;
};

struct transfer
{
    hsd::usize from;
    hsd::usize to;
    hsd::i64 amount;
};

// Two transfers that move money in opposite directions would deadlock if each
// locked its source first. Always locking the lower index first prevents it.
void move_money(hsd::vector<account*>& accounts, const transfer& job)
{
    hsd::usize first = job.from < job.to ? job.from : job.to;
    hsd::usize second = job.from < job.to ? job.to : job.from;

    hsd::unique_lock<hsd::mutex> first_lock{hsd::dont_lock, accounts[first]->mutex};
    hsd::unique_lock<hsd::mutex> second_lock{hsd::dont_lock, accounts[second]->mutex};
    first_lock.lock().unwrap();
    second_lock.lock().unwrap();

    accounts[job.from]->balance -= job.amount;
    accounts[job.to]->balance += job.amount;
}

int main()
{
    constexpr hsd::usize account_count = 4;
    constexpr hsd::usize worker_count = 4;
    constexpr hsd::i64 start_balance = 100000;

    // Mutexes cannot move, so the accounts live separately and are reached by pointer.
    hsd::vector<account> storage;
    storage.resize(account_count);
    hsd::vector<account*> accounts;

    for (hsd::usize i = 0; i < account_count; ++i)
    {
        storage[i].balance = start_balance;
        accounts.push_back(&storage[i]);
    }

    // Each worker runs a fixed schedule, and the amounts are small enough
    // that no account can run dry, so the final balances never depend on timing.
    hsd::vector<hsd::thread> workers;

    for (hsd::usize w = 0; w < worker_count; ++w)
    {
        workers.emplace_back([&accounts, w]
        {
            for (hsd::usize i = 0; i < 500; ++i)
            {
                transfer job{
                    (w + i) % account_count,
                    (w * 2 + i * 3 + 1) % account_count,
                    static_cast<hsd::i64>((i + w) % 7 + 1)
                };

                if (job.from != job.to)
                {
                    move_money(accounts, job);
                }
            }
        });
    }

    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    hsd::i64 total = 0;

    for (hsd::usize i = 0; i < account_count; ++i)
    {
        hsd::println("account {}: {}"_fmt, i, accounts[i]->balance);
        total += accounts[i]->balance;
    }

    hsd::println("total: {}"_fmt, total);
    hsd::println("money conserved: {}"_fmt,
        total == start_balance * static_cast<hsd::i64>(account_count) ? "true" : "false");

    return 0;
}
