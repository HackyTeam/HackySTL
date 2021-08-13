#include <SharedPtr.hpp>
#include <Io.hpp>
#include <Thread.hpp>

struct S
{
    int _a;
    char _b;
    float _c;
    const char* _d;

    constexpr S(int a, char b, float c, const char* d)
        : _a{a}, _b{b}, _c{c}, _d{d}
    {}
};

static inline hsd::atomic_flag lock{};
static inline hsd::safe_shared_ptr<S> elm = 
    hsd::make_safe_shared<S>(12, 'c', 4.3f, "str");

static void print(hsd::safe_shared_ptr<S> ptr)
{
    hsd_println(
        "{}\n{}\n{}\n{}", 
        ptr->_a, ptr->_b, 
        ptr->_c, ptr->_d
    );
}


void thread_func()
{
    for (hsd::i32 count = 0; count < 1; count++)
    {
        // spin until the lock is acquired
        while (lock.test_and_set(hsd::memory_order_acquire))
            ;

        print(elm);
        lock.clear(hsd::memory_order_release);
    }
}

int main()
{
    hsd::vector<hsd::thread> vec;

    for (int index = 0; index < 3; index++)
    {
        vec.emplace_back(thread_func);
    }

    for (auto& thread_val : vec)
    {
        thread_val.join().unwrap();
    }
}