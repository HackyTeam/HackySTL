// Title: Thread pipeline
// Level: Advanced
// Shows: three stages on separate threads connected by mutex and condition variable queues.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/ConditionVariable.hpp>
#include <Threading/Lock.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

// A queue that one stage fills and the next stage drains.
// close() tells the reader that no more values will arrive.
class channel
{
private:
    hsd::mutex _mutex{};
    hsd::condition_variable _cond_var{};
    hsd::vector<hsd::i32> _items{};
    hsd::usize _head = 0;
    bool _closed = false;

public:
    void send(hsd::i32 value)
    {
        {
            hsd::unique_lock<hsd::mutex> lock{_mutex};
            _items.push_back(value);
        }

        _cond_var.notify_one();
    }

    void close()
    {
        {
            hsd::unique_lock<hsd::mutex> lock{_mutex};
            _closed = true;
        }

        _cond_var.notify_all();
    }

    // Returns false once the channel is closed and drained.
    bool receive(hsd::i32& value)
    {
        hsd::unique_lock<hsd::mutex> lock{_mutex};
        _cond_var.wait(lock, [this] { return _head < _items.size() || _closed; });

        if (_head < _items.size())
        {
            value = _items[_head++];
            return true;
        }

        return false;
    }
};

int main()
{
    channel numbers;
    channel squares;
    hsd::vector<hsd::i32> results;
    hsd::i32 total = 0;

    // Stage 1 generates the numbers 1 to 10.
    hsd::thread generate{[&numbers]
    {
        for (hsd::i32 i = 1; i <= 10; ++i)
        {
            numbers.send(i);
        }

        numbers.close();
    }};

    // Stage 2 squares everything it receives and passes it on.
    hsd::thread square{[&numbers, &squares]
    {
        hsd::i32 value = 0;

        while (numbers.receive(value))
        {
            squares.send(value * value);
        }

        squares.close();
    }};

    // Stage 3 collects the values in arrival order. Only this thread touches
    // results and total until it is joined, so main reads them safely afterwards.
    hsd::thread collect{[&squares, &results, &total]
    {
        hsd::i32 value = 0;

        while (squares.receive(value))
        {
            results.push_back(value);
            total += value;
        }
    }};

    generate.join().unwrap();
    square.join().unwrap();
    collect.join().unwrap();

    hsd::print("squares:"_fmt);

    for (hsd::usize i = 0; i < results.size(); ++i)
    {
        hsd::print(" {}"_fmt, results[i]);
    }

    hsd::println(""_fmt);
    hsd::println("total: {}"_fmt, total);
    return 0;
}
