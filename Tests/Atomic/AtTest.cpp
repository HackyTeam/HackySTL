#include <Thread.hpp>
#include <Vector.hpp>
#include <Io.hpp>
#include <Atomic.hpp>
 
hsd::atomic_flag lock{};
 
void f(hsd::i32 n)
{
    for (hsd::i32 cnt = 0; cnt < 100; ++cnt) {
        while(hsd::atomic_flag_test_and_set_explicit(&lock, hsd::memory_order_acquire))
             ; // spin until the lock is acquired
        hsd::io::print<"Output from thread {}\n">(n);
        hsd::atomic_flag_clear_explicit(&lock, hsd::memory_order_release);
    }
}
 
int main()
{
    hsd::vector<hsd::thread> v;
    for (int n = 0; n < 10; ++n) {
        v.emplace_back(f, n);
    }
    for (auto& t : v) {
        t.join();
    }
}