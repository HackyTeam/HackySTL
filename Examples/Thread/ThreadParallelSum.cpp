// Title: Parallel sum
// Level: Intermediate
// Shows: a vector of threads, per-thread result slots, and joining in order.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

int main()
{
    constexpr hsd::usize worker_count = 4;
    constexpr hsd::usize item_count = 1000;

    hsd::vector<hsd::i64> numbers;
    numbers.reserve(item_count);

    for (hsd::usize i = 1; i <= item_count; ++i)
    {
        numbers.push_back(static_cast<hsd::i64>(i));
    }

    // Every worker owns one slot, so no two threads write the same memory.
    hsd::vector<hsd::i64> partial;
    partial.resize(worker_count);

    // hsd::thread is move-only, but emplace_back builds it in place.
    hsd::vector<hsd::thread> workers;
    workers.reserve(worker_count);
    constexpr hsd::usize chunk = item_count / worker_count;

    for (hsd::usize w = 0; w < worker_count; ++w)
    {
        workers.emplace_back([&numbers, &partial, w]
        {
            hsd::i64 sum = 0;

            for (hsd::usize i = w * chunk; i < (w + 1) * chunk; ++i)
            {
                sum += numbers[i];
            }

            partial[w] = sum;
        });
    }

    // Joining in index order keeps the printed order fixed.
    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    hsd::i64 total = 0;

    for (hsd::usize w = 0; w < worker_count; ++w)
    {
        hsd::println("chunk {}: {}"_fmt, w, partial[w]);
        total += partial[w];
    }

    hsd::println("total: {}"_fmt, total);
    return 0;
}
