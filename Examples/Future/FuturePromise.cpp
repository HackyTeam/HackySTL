// Title: Promise and future
// Level: Basic
// Shows: handing a value from a worker thread to main with promise and future.

#include <Serialize/Io.hpp>
#include <Threading/Future.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

int main()
{
    // A promise is the writing end and its future is the reading end.
    hsd::promise<hsd::i32> promise;
    hsd::future<hsd::i32> future = promise.get_future();

    hsd::thread worker{[&promise]
    {
        promise.set_value(42);
    }};

    // wait() blocks until a value is set. get() does not block by itself,
    // so call wait() first.
    future.wait();
    auto& result = future.get();
    hsd::println("is_ok: {}"_fmt, result.is_ok() ? "true" : "false");
    hsd::println("value: {}"_fmt, result.unwrap());
    worker.join().unwrap();

    // A promise can report a failure instead of a value.
    hsd::promise<hsd::i32> failing;
    hsd::future<hsd::i32> failing_future = failing.get_future();

    hsd::thread failing_worker{[&failing]
    {
        failing.set_error(hsd::runtime_error{"the worker gave up"});
    }};

    failing_future.wait();
    auto& failure = failing_future.get();
    hsd::println("is_ok: {}"_fmt, failure.is_ok() ? "true" : "false");

    if (!failure.is_ok())
    {
        hsd::println("error: {}"_fmt, failure.unwrap_err().pretty_error());
    }

    failing_worker.join().unwrap();
    return 0;
}
