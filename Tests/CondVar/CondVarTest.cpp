#include <Threading/Thread.hpp>
#include <Threading/ConditionVariable.hpp>
#include <cassert>
#include <Serialize/Io.hpp>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

hsd::mutex mutex;
hsd::condition_variable cond_var;
hsd::string data;
bool ready = false;
bool processed = false;

using namespace hsd::format_literals;

static void test_condition_variable_against_std()
{
    std::mutex standard_mutex;
    std::condition_variable standard_cond_var;
    std::string standard_data{"Example data"};
    bool standard_ready = false;
    bool standard_processed = false;

    std::thread standard_worker([&]
    {
        std::unique_lock<std::mutex> lock{standard_mutex};
        standard_cond_var.wait(lock, [&] { return standard_ready; });
        standard_data += " after processing";
        standard_processed = true;
        lock.unlock();
        standard_cond_var.notify_one();
    });

    {
        std::lock_guard<std::mutex> lock{standard_mutex};
        standard_ready = true;
    }
    standard_cond_var.notify_one();

    {
        std::unique_lock<std::mutex> lock{standard_mutex};
        standard_cond_var.wait(lock, [&] { return standard_processed; });
    }
    standard_worker.join();

    assert(data == hsd::string{standard_data.c_str()});
    assert(processed == standard_processed);
}

void worker_thread()
{
    // Wait until main() sends data
    hsd::unique_lock<hsd::mutex> lock{mutex};
    cond_var.wait(lock, []{ return ready; });

    // after the wait, we own the lock.
    data += " after processing";

    // Send data back to main()
    processed = true;

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lock.unlock().unwrap();
    cond_var.notify_one();
}

int main()
{
    hsd::thread worker{worker_thread};
 
    data = "Example data";
    // send data to the worker thread
    {
        hsd::unique_lock<hsd::mutex> lock{mutex};
        ready = true;
    }
    
    cond_var.notify_one();
 
    // wait for the worker
    {
        hsd::unique_lock<hsd::mutex> lock{mutex};
        cond_var.wait(lock, []{ return processed; });
    }
    
    assert(data == hsd::string{"Example data after processing"});
    assert(processed);
    worker.join().unwrap();
    test_condition_variable_against_std();
}