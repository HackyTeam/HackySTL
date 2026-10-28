#include <Threading/ConditionVariable.hpp>
#include <Threading/Lock.hpp>
#include <Threading/MonotonicClock.hpp>
#include <Threading/Time.hpp>
#include <cassert>

int main()
{
    hsd::mutex mtx;
    hsd::condition_variable cv;
    hsd::unique_lock<hsd::mutex> lock{mtx};

    // Nothing notifies, so a 50 ms relative wait must last about 50 ms. The
    // deadline is on the wall clock while the measurement is monotonic, so a
    // small margin absorbs clock slew. A wait that returns at once measures
    // near zero and still fails.
    constexpr hsd::u64 min_wait_ns = 40'000'000;
    const hsd::precise_clock timeout{timespec{0, 50'000'000}};

    const hsd::u64 start = hsd::monotonic_clock::now_ns();
    const hsd::cv_status status = cv.wait_for(lock, timeout);
    const hsd::u64 elapsed = hsd::monotonic_clock::now_ns() - start;

    assert(status == hsd::cv_status::timeout);
    assert(elapsed >= min_wait_ns);

    // The predicate overload keeps waiting until the deadline and then reports
    // the predicate's value, which is false here.
    const hsd::u64 pred_start = hsd::monotonic_clock::now_ns();
    const bool satisfied = cv.wait_for(lock, timeout, [] { return false; });
    const hsd::u64 pred_elapsed = hsd::monotonic_clock::now_ns() - pred_start;

    assert(!satisfied);
    assert(pred_elapsed >= min_wait_ns);

    return 0;
}
