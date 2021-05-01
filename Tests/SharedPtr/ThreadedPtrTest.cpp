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
    hsd::safe_make_shared<S>(12, 'c', 4.3f, "str");

static void print(hsd::safe_shared_ptr<S> ptr)
{
    hsd::io::print<"{}\n{}\n{}\n{}\n">(
        ptr->_a, ptr->_b, ptr->_c, ptr->_d
    );
}


void thread_func()
{
    for (hsd::i32 count = 0; count < 1; count++)
    {
        // spin until the lock is acquired
        while(hsd::atomic_flag_test_and_set_explicit(&lock, hsd::memory_order_acquire));

        print(elm);
        hsd::atomic_flag_clear_explicit(
            &lock, hsd::memory_order_release
        );
    }
}

int main()
{
    hsd::vector<hsd::thread> vec;

    for (int index = 0; index < 10; index++)
    {
        vec.emplace_back(thread_func);
    }

    for (auto& thread_val : vec)
    {
        thread_val.join();
    }
}