#include <Thread.hpp>
#include <Vector.hpp>
#include <Io.hpp>
#include <Atomic.hpp>
 
hsd::atomic_flag lock{};
 
void thread_func(hsd::i32 thread_index)
{
    for (hsd::i32 count = 0; count < 100; count++)
    {
        // spin until the lock is acquired
        while(hsd::atomic_flag_test_and_set_explicit(&lock, hsd::memory_order_acquire));

        hsd::io::print<"Output from thread {}\n">(thread_index);
        hsd::atomic_flag_clear_explicit(&lock, hsd::memory_order_release);
    }
}
 
int main()
{
    hsd::vector<hsd::thread> vec;

    for (int index = 0; index < 10; index++)
    {
        vec.emplace_back(thread_func, index);
    }

    for (auto& thread_val : vec)
    {
        thread_val.join();
    }
}