// Title: Fan out with packaged tasks
// Level: Intermediate
// Shows: packaged_task on several threads, with futures collected in order.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Future.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

int main()
{
    constexpr hsd::i32 task_count = 5;

    hsd::vector<hsd::future<hsd::i32>> futures;
    hsd::vector<hsd::thread> threads;

    for (hsd::i32 t = 0; t < task_count; ++t)
    {
        // The task wraps a callable. A callable that returns a value
        // fills the future when the task runs.
        hsd::packaged_task task = {[t]
        {
            hsd::i32 sum = 0;

            for (hsd::i32 i = 1; i <= (t + 1) * 10; ++i)
            {
                sum += i * i;
            }

            return sum;
        }};

        // Take the future before the task moves into its thread.
        futures.push_back(task.get_future());
        threads.emplace_back(hsd::move(task));
    }

    // Waiting in index order prints the results in a fixed order,
    // however the threads were scheduled.
    hsd::i32 grand_total = 0;

    for (hsd::i32 t = 0; t < task_count; ++t)
    {
        futures[t].wait();
        auto& result = futures[t].get();

        if (result.is_ok())
        {
            hsd::println("squares up to {}: {}"_fmt, (t + 1) * 10, result.unwrap());
            grand_total += result.unwrap();
        }
    }

    for (auto& thread : threads)
    {
        thread.join().unwrap();
    }

    hsd::println("total: {}"_fmt, grand_total);
    return 0;
}
