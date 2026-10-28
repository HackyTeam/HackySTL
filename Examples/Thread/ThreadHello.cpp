// Title: Thread hello
// Level: Basic
// Shows: starting a hsd::thread, passing arguments, joining, and the joinable state.

#include <Threading/Thread.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// The worker never prints: it writes into a slot that main owns.
void write_greeting(hsd::i32* slot, hsd::i32 value)
{
    *slot = value * 2;
}

int main()
{
    hsd::i32 result = 0;

    // Arguments are copied into the thread, so a pointer is how
    // the worker hands a value back to the caller.
    hsd::thread worker{write_greeting, &result, 21};
    hsd::println("joinable before join: {}"_fmt, worker.joinable() ? "true" : "false");

    // join() returns an option_err, and unwrap() is fine on the first call.
    worker.join().unwrap();
    hsd::println("joinable after join: {}"_fmt, worker.joinable() ? "true" : "false");
    hsd::println("worker wrote: {}"_fmt, result);

    // A thread that is no longer joinable cannot be joined again.
    hsd::println("second join ok: {}"_fmt, worker.join().is_ok() ? "true" : "false");

    // A lambda that captures by reference can also be the thread body.
    hsd::i32 from_lambda = 0;
    hsd::thread lambda_thread{[&from_lambda] { from_lambda = 7; }};

    // Ownership of a thread moves, and the source stops being joinable.
    hsd::thread moved{hsd::move(lambda_thread)};
    hsd::println("source joinable after move: {}"_fmt, lambda_thread.joinable() ? "true" : "false");
    moved.join().unwrap();
    hsd::println("lambda wrote: {}"_fmt, from_lambda);

    // The worker's id differs from the id of the main thread.
    hsd::id main_id = hsd::this_thread::get_id();
    hsd::id worker_id = main_id;
    hsd::thread id_thread{[&worker_id] { worker_id = hsd::this_thread::get_id(); }};
    id_thread.join().unwrap();
    hsd::println("ids differ: {}"_fmt, worker_id == main_id ? "false" : "true");

    return 0;
}
