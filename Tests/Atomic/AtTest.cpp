#include <Thread.hpp>
#include <Vector.hpp>
#include <Io.hpp>
#include <Atomic.hpp>
 
hsd::atomic_flag lock{};
 
void thread_func(hsd::i32 thread_index)
{
    using namespace hsd::format_literals;

    for (hsd::i32 count = 0; count < 100; count++)
    {
        // spin until the lock is acquired
        while (lock.test_and_set(hsd::memory_order_acquire));

        hsd::println("Output from thread {}"_fmt, thread_index);
        lock.clear(hsd::memory_order_release);
    }
}
 
int main()
{
    hsd::vector<hsd::thread> vec;

    for (int index = 0; index < 3; index++)
    {
        vec.emplace_back(thread_func, index);
    }

    for (auto& thread_val : vec)
    {
        thread_val.join().unwrap();
    }
}