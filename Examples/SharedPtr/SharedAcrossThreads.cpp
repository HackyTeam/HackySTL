// Title: Shared across threads
// Level: Advanced
// Shows: safe_shared_ptr copied into worker threads, atomic reference counts under contention, deterministic results.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>
#include <Wrappers/SharedPtr.hpp>

using namespace hsd::format_literals;

// Read-only data shared by every worker.
struct dataset
{
    hsd::vector<hsd::i64> numbers;
};

int main()
{
    constexpr hsd::usize worker_count = 4;
    constexpr hsd::usize item_count = 1000;
    constexpr hsd::usize copies_per_worker = 5000;

    auto data = hsd::make_safe_shared<dataset>();

    for (hsd::usize index = 1; index <= item_count; ++index)
    {
        data->numbers.push_back(static_cast<hsd::i64>(index));
    }

    hsd::println("owners before start: {}"_fmt, data.get_count());

    // One result slot per worker, so no two threads write the same memory.
    hsd::vector<hsd::i64> sums;
    sums.resize(worker_count);
    hsd::vector<hsd::usize> observed_unique;
    observed_unique.resize(worker_count);

    {
        hsd::vector<hsd::thread> workers;
        workers.reserve(worker_count);
        constexpr hsd::usize chunk = item_count / worker_count;

        for (hsd::usize worker = 0; worker < worker_count; ++worker)
        {
            // Every worker holds its own owner. The reference count is atomic,
            // so the owners can be copied and destroyed from all threads at once.
            workers.emplace_back([owner = data, &sums, &observed_unique, worker]() mutable
            {
                hsd::i64 sum = 0;

                for (hsd::usize index = worker * chunk; index < (worker + 1) * chunk; ++index)
                {
                    sum += owner->numbers[index];
                }

                // Copy and drop the owner many times to hammer the count.
                for (hsd::usize round = 0; round < copies_per_worker; ++round)
                {
                    auto copy = owner;
                    sum += copy->numbers[0] - 1;
                }

                sums[worker] = sum;
                observed_unique[worker] = owner.is_unique() ? 1u : 0u;
            });
        }

        // Join in index order. The workers never print: main does all the output.
        for (auto& worker : workers)
        {
            worker.join().unwrap();
        }
    }

    hsd::i64 total = 0;

    for (hsd::usize worker = 0; worker < worker_count; ++worker)
    {
        hsd::println("worker {} sum: {}"_fmt, worker, sums[worker]);
        total += sums[worker];
    }

    hsd::println("total: {}"_fmt, total);

    // The main thread still holds its own owner, so no worker ever saw a count of one.
    hsd::usize unique_seen = 0;

    for (hsd::usize worker = 0; worker < worker_count; ++worker)
    {
        unique_seen += observed_unique[worker];
    }

    hsd::println("workers that were the only owner: {}"_fmt, unique_seen);

    // The worker threads are gone, and with them their owners.
    hsd::println("owners after join: {} unique {}"_fmt,
        data.get_count(), data.is_unique() ? "yes" : "no");
    return 0;
}
