#include <Lock.hpp>
#include <Thread.hpp>
#include <UnorderedMap.hpp>
#include <String.hpp>
#include <Io.hpp>

class FutexTest
{
private:
    static inline hsd::unordered_map<hsd::string, hsd::string> g_pages;
    static inline hsd::futex g_pages_futex;

public:
    static void save_page(const hsd::string& url)
    {
        // simulate a long page fetch
        hsd::this_thread::sleep_for(2).unwrap();
        hsd::string result = "fake content";

        hsd::unique_lock lock{g_pages_futex};
        g_pages[url] = result;
    }

    static void test()
    {
        hsd_println("FutexTest::test():");
        hsd::thread t1{save_page, "http://foo"};
        hsd::thread t2{save_page, "http://bar"};
        t1.join().unwrap();
        t2.join().unwrap();

        // safe to access g_pages without lock now, as the threads are joined
        for (const auto& pair : g_pages)
        {
            hsd_println("{} => {}", pair.first, pair.second);
        }
    }
};

class MutexTest
{
private:
    static inline hsd::unordered_map<hsd::string, hsd::string> g_pages;
    static inline hsd::mutex g_pages_mutex;

public:
    static void save_page(const hsd::string& url)
    {
        // simulate a long page fetch
        hsd::this_thread::sleep_for(2).unwrap();
        hsd::string result = "fake content";

        hsd::unique_lock lock{g_pages_mutex};
        g_pages[url] = result;
    }

    static void test()
    {
        hsd_println("MutexTest::test():");
        hsd::thread t1{save_page, "http://foo"};
        hsd::thread t2{save_page, "http://bar"};
        t1.join().unwrap();
        t2.join().unwrap();

        // safe to access g_pages without lock now, as the threads are joined
        for (const auto& pair : g_pages)
        {
            hsd_println("{} => {}", pair.first, pair.second);
        }
    }
};

class SpinTest
{
private:
    static inline hsd::unordered_map<hsd::string, hsd::string> g_pages;
    static inline hsd::spin g_pages_spin;

public:
    static void save_page(const hsd::string& url)
    {
        // simulate a long page fetch
        hsd::this_thread::sleep_for(2).unwrap();
        hsd::string result = "fake content";

        hsd::unique_lock lock{g_pages_spin};
        g_pages[url] = result;
    }

    static void test()
    {
        hsd_println("SpinTest::test():");
        hsd::thread t1{save_page, "http://foo"};
        hsd::thread t2{save_page, "http://bar"};
        t1.join().unwrap();
        t2.join().unwrap();

        // safe to access g_pages without lock now, as the threads are joined
        for (const auto& pair : g_pages)
        {
            hsd_println("{} => {}", pair.first, pair.second);
        }
    }
};

int main()
{
    SpinTest::test();
    MutexTest::test();
    FutexTest::test();
}