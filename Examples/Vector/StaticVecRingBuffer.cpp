// Title: Ring buffer on static_vector
// Level: Advanced
// Shows: static_vector with fixed capacity, a ring buffer built on it, and constexpr use checked by static_assert.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// A fixed-capacity ring buffer. The storage is a static_vector that is filled
// to N slots once, so no heap memory is ever allocated.
template <typename T, hsd::usize N>
class ring_buffer
{
private:
    hsd::static_vector<T, N> _slots;
    hsd::usize _head = 0;
    hsd::usize _count = 0;

public:
    constexpr ring_buffer()
    {
        _slots.resize(N);
    }

    // Pushes a value. When the buffer is full the oldest value is dropped.
    // Returns true when a value was dropped.
    constexpr bool push(const T& value)
    {
        bool dropped = false;

        if (_count == N)
        {
            _head = (_head + 1) % N;
            --_count;
            dropped = true;
        }

        _slots[(_head + _count) % N] = value;
        ++_count;
        return dropped;
    }

    // Removes the oldest value; returns false if the buffer is empty.
    constexpr bool pop(T& out)
    {
        if (_count == 0)
        {
            return false;
        }

        out = _slots[_head];
        _head = (_head + 1) % N;
        --_count;
        return true;
    }

    constexpr hsd::usize size() const
    {
        return _count;
    }

    constexpr const T& operator[](hsd::usize index) const
    {
        return _slots[(_head + index) % N];
    }
};

// Everything below also runs at compile time.
constexpr int sum_of_last_three()
{
    ring_buffer<int, 3> buffer;
    for (int value = 1; value <= 10; ++value)
    {
        buffer.push(value);
    }

    int sum = 0;
    for (hsd::usize i = 0; i < buffer.size(); ++i)
    {
        sum += buffer[i];
    }
    return sum;
}

static_assert(sum_of_last_three() == 8 + 9 + 10);

constexpr hsd::usize static_vector_size()
{
    hsd::static_vector<int, 8> values;
    values.push_back(1);
    values.push_back(2);
    values.push_back(3);
    values.pop_back();
    return values.size();
}

static_assert(static_vector_size() == 2);

void print_buffer(const ring_buffer<hsd::i32, 4>& buffer)
{
    hsd::print("holds {}:"_fmt, buffer.size());
    for (hsd::usize i = 0; i < buffer.size(); ++i)
    {
        hsd::print(" {}"_fmt, buffer[i]);
    }
    hsd::println(""_fmt);
}

int main()
{
    // A static_vector keeps its elements inline, with a capacity fixed by the type.
    hsd::static_vector<hsd::i32, 4> plain;
    plain.push_back(7);
    plain.push_back(8);
    hsd::println("static_vector size={} capacity={}"_fmt, plain.size(), plain.capacity());

    ring_buffer<hsd::i32, 4> buffer;
    for (hsd::i32 value = 1; value <= 6; ++value)
    {
        bool dropped = buffer.push(value * 10);
        hsd::println("push {}{}"_fmt, value * 10, dropped ? " (dropped oldest)" : "");
    }
    print_buffer(buffer);

    hsd::i32 oldest = 0;
    if (buffer.pop(oldest))
    {
        hsd::println("popped {}"_fmt, oldest);
    }
    buffer.push(70);
    print_buffer(buffer);

    // Drain it and show that popping an empty buffer reports failure.
    hsd::i32 drained = 0;
    hsd::usize pops = 0;
    while (buffer.pop(drained))
    {
        ++pops;
    }
    hsd::println("drained {} values, last {}"_fmt, pops, drained);
    hsd::println("pop on empty: {}"_fmt, buffer.pop(drained) ? "true" : "false");

    hsd::println("compile-time sum of last three: {}"_fmt, sum_of_last_three());

    return 0;
}
