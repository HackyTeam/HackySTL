#include <Wrappers/SharedPtr.hpp>
#include <atomic>
#include <barrier>
#include <cassert>
#include <thread>
#include <vector>

static std::atomic<long> constructed{0};
static std::atomic<long> destroyed{0};

struct Tracked
{
    int value = 7;

    Tracked()
    {
        ++constructed;
    }

    ~Tracked()
    {
        ++destroyed;
    }
};

// Sense-reversing barrier for exactly two threads. Both are released within a few
// nanoseconds of each other, which is what makes the race likely.
struct SpinBarrier2
{
    std::atomic<int> count{0};
    std::atomic<int> sense{0};

    void wait(int& local)
    {
        local ^= 1;

        if (count.fetch_add(1) == 1)
        {
            count.store(0);
            sense.store(local);
        }
        else
        {
            while (sense.load() != local)
            {
            }
        }
    }
};

int main()
{
    constexpr long object_count = 200000;
    constexpr long chunk = 4096;
    constexpr long chunks = (object_count + chunk - 1) / chunk;

    std::vector<hsd::safe_shared_ptr<Tracked>> first(chunk);
    std::vector<hsd::safe_shared_ptr<Tracked>> second(chunk);
    std::barrier start(3);
    std::barrier finish(3);
    SpinBarrier2 pair_barrier;

    auto worker = [&](std::vector<hsd::safe_shared_ptr<Tracked>>* owners)
    {
        int local = 0;

        for (long c = 0; c < chunks; ++c)
        {
            start.arrive_and_wait();

            for (long i = 0; i < chunk; ++i)
            {
                pair_barrier.wait(local);
                (*owners)[i] = nullptr;
            }

            finish.arrive_and_wait();
        }
    };

    std::thread first_thread(worker, &first);
    std::thread second_thread(worker, &second);

    for (long c = 0; c < chunks; ++c)
    {
        for (long i = 0; i < chunk; ++i)
        {
            auto owner = hsd::make_safe_shared<Tracked>();
            second[i] = owner;
            first[i] = hsd::move(owner);
        }

        start.arrive_and_wait();
        finish.arrive_and_wait();
    }

    first_thread.join();
    second_thread.join();

    assert(constructed.load() == destroyed.load());
    assert(constructed.load() >= object_count);
    return 0;
}
