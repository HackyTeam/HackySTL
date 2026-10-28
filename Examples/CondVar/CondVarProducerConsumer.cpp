// Title: Producer and consumers
// Level: Intermediate
// Shows: a bounded queue with two condition variables, several consumers, and a done flag.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/ConditionVariable.hpp>
#include <Threading/Lock.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

// A fixed-size ring buffer. The producer waits while it is full,
// and consumers wait while it is empty.
class bounded_queue
{
private:
    hsd::mutex _mutex{};
    hsd::condition_variable _not_full{};
    hsd::condition_variable _not_empty{};
    hsd::i32 _items[4]{};
    hsd::usize _head = 0;
    hsd::usize _count = 0;
    bool _done = false;

public:
    void push(hsd::i32 value)
    {
        hsd::unique_lock<hsd::mutex> lock{_mutex};
        _not_full.wait(lock, [this] { return _count < 4; });
        _items[(_head + _count) % 4] = value;
        ++_count;
        lock.unlock().unwrap();
        _not_empty.notify_one();
    }

    void finish()
    {
        {
            hsd::unique_lock<hsd::mutex> lock{_mutex};
            _done = true;
        }

        // Every sleeping consumer must see the flag, so wake all of them.
        _not_empty.notify_all();
    }

    bool pop(hsd::i32& value)
    {
        hsd::unique_lock<hsd::mutex> lock{_mutex};
        _not_empty.wait(lock, [this] { return _count > 0 || _done; });

        if (_count == 0)
        {
            return false;
        }

        value = _items[_head];
        _head = (_head + 1) % 4;
        --_count;
        lock.unlock().unwrap();
        _not_full.notify_one();
        return true;
    }
};

int main()
{
    constexpr hsd::usize consumer_count = 2;
    constexpr hsd::i32 item_count = 50;

    bounded_queue queue;
    hsd::vector<hsd::i64> sums;
    hsd::vector<hsd::i32> counts;
    sums.resize(consumer_count);
    counts.resize(consumer_count);

    hsd::vector<hsd::thread> consumers;

    for (hsd::usize c = 0; c < consumer_count; ++c)
    {
        consumers.emplace_back([&queue, &sums, &counts, c]
        {
            hsd::i32 value = 0;

            while (queue.pop(value))
            {
                sums[c] += value;
                ++counts[c];
            }
        });
    }

    hsd::thread producer{[&queue]
    {
        for (hsd::i32 i = 1; i <= item_count; ++i)
        {
            queue.push(i);
        }

        queue.finish();
    }};

    producer.join().unwrap();

    for (auto& consumer : consumers)
    {
        consumer.join().unwrap();
    }

    // How the work splits between consumers depends on scheduling,
    // so only the combined totals are printed.
    hsd::i64 total = 0;
    hsd::i32 consumed = 0;

    for (hsd::usize c = 0; c < consumer_count; ++c)
    {
        total += sums[c];
        consumed += counts[c];
    }

    hsd::println("produced: {}"_fmt, item_count);
    hsd::println("consumed: {}"_fmt, consumed);
    hsd::println("sum: {}"_fmt, total);
    return 0;
}
