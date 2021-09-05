#include <Lock.hpp>
#include <Thread.hpp>
#include <UnorderedMap.hpp>
#include <String.hpp>
#include <Io.hpp>

hsd::unordered_map<hsd::string, hsd::string> g_pages;
hsd::futex g_pages_futex;

void save_page(const hsd::string& url)
{
    // simulate a long page fetch
    hsd::this_thread::sleep_for(2).unwrap();
    hsd::string result = "fake content";

    hsd::unique_lock<hsd::futex> lock{g_pages_futex};
    g_pages[url] = result;
}

int main()
{
    hsd::thread t1(save_page, "http://foo");
    hsd::thread t2(save_page, "http://bar");
    t1.join().unwrap();
    t2.join().unwrap();

    // safe to access g_pages without lock now, as the threads are joined
    for (const auto& pair : g_pages)
    {
        hsd_println("{} => {}", pair.first, pair.second);
    }
}