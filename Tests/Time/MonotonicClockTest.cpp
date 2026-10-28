#include <Threading/MonotonicClock.hpp>
#include <Threading/Thread.hpp>
#include <cassert>

int main()
{
    // Successive readings never decrease.
    hsd::u64 previous = hsd::monotonic_clock::now_ns();
    for (int i = 0; i < 10'000; ++i)
    {
        const hsd::u64 current = hsd::monotonic_clock::now_ns();
        assert(current >= previous);
        previous = current;
    }

    // Wall time spent sleeping must be visible in the measured duration.
    const hsd::u64 start = hsd::monotonic_clock::now_ns();
    hsd::this_thread::sleep_for(0.02f).unwrap();
    const hsd::u64 elapsed = hsd::monotonic_clock::now_ns() - start;
    assert(elapsed >= 20'000'000);

    return 0;
}
