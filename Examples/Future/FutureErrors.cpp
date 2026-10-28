// Title: Errors through futures
// Level: Advanced
// Shows: tasks returning result, error propagation, option_err tasks, and timed waits.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Future.hpp>
#include <Threading/Thread.hpp>
#include <Threading/Time.hpp>

using namespace hsd::format_literals;
using namespace hsd::precise_clock_literals;

using int_result = hsd::result<hsd::i32, hsd::runtime_error>;

// When the task returns a result, an Err becomes the future's error.
int_result divide(hsd::i32 a, hsd::i32 b)
{
    if (b == 0)
    {
        return hsd::runtime_error{"division by zero"};
    }

    return a / b;
}

hsd::option_err<hsd::runtime_error> check_even(hsd::i32 value)
{
    if (value % 2 != 0)
    {
        return hsd::runtime_error{"value is odd"};
    }

    return {};
}

int main()
{
    // Three divisions, one of them invalid.
    hsd::i32 numerators[3] = {100, 7, 81};
    hsd::i32 denominators[3] = {5, 0, 9};
    hsd::vector<hsd::future<hsd::i32>> futures;
    hsd::vector<hsd::thread> threads;

    for (hsd::i32 i = 0; i < 3; ++i)
    {
        hsd::packaged_task task = {divide};
        futures.push_back(task.get_future());
        threads.emplace_back(hsd::move(task), numerators[i], denominators[i]);
    }

    for (hsd::i32 i = 0; i < 3; ++i)
    {
        futures[i].wait();
        auto& result = futures[i].get();

        if (result.is_ok())
        {
            hsd::println("{} / {} = {}"_fmt, numerators[i], denominators[i], result.unwrap());
        }
        else
        {
            hsd::println("{} / {} failed: {}"_fmt, numerators[i], denominators[i],
                result.unwrap_err().pretty_error());
        }
    }

    for (auto& thread : threads)
    {
        thread.join().unwrap();
    }

    // A task that returns option_err has no value, only success or an error.
    hsd::packaged_task even_task = {check_even};
    hsd::packaged_task odd_task = {check_even};
    auto even_future = even_task.get_future();
    auto odd_future = odd_task.get_future();
    hsd::thread even_thread{hsd::move(even_task), 8};
    hsd::thread odd_thread{hsd::move(odd_task), 9};

    even_future.wait();
    odd_future.wait();
    hsd::println("8 is even: {}"_fmt, even_future.get().is_ok() ? "true" : "false");
    hsd::println("9 is even: {}"_fmt, odd_future.get().is_ok() ? "true" : "false");
    even_thread.join().unwrap();
    odd_thread.join().unwrap();

    // A promise nobody has fulfilled yet still holds its starting error,
    // and a timed wait gives up instead of blocking forever.
    hsd::promise<hsd::i32> pending;
    hsd::future<hsd::i32> pending_future = pending.get_future();
    pending_future.wait_for(30_ms);
    hsd::println("ready after 30 ms: {}"_fmt, pending_future.is_valid() ? "true" : "false");
    hsd::println("starting state: {}"_fmt, pending_future.get().unwrap_err().pretty_error());

    // Once the value arrives, the same future is ready.
    pending.set_value(5);
    pending_future.wait_for(30_ms);
    hsd::println("ready after set_value: {}"_fmt, pending_future.is_valid() ? "true" : "false");
    hsd::println("value: {}"_fmt, pending_future.get().unwrap());
    return 0;
}
