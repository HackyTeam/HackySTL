#include <Thread.hpp>
#include <ConditionVariable.hpp>
#include <Io.hpp>

hsd::mutex mutex;
hsd::condition_variable cond_var;
hsd::string data;
bool ready = false;
bool processed = false;

using namespace hsd::format_literals;

void worker_thread()
{
    // Wait until main() sends data
    hsd::unique_lock<hsd::mutex> lock{mutex};
    cond_var.wait(lock, []{ return ready; });

    // after the wait, we own the lock.
    hsd::println("Worker thread is processing data"_fmt);
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
        hsd::println("main() signals data ready for processing"_fmt);
    }
    
    cond_var.notify_one();
 
    // wait for the worker
    {
        hsd::unique_lock<hsd::mutex> lock{mutex};
        cond_var.wait(lock, []{ return processed; });
    }
    
    hsd::println("Back in main(), data = {}"_fmt, data);
    worker.join().unwrap();
}