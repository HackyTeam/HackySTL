#include <Threading/Thread.hpp>
#include <Threading/Future.hpp>
#include <cassert>
#include <future>

hsd::option_err<hsd::runtime_error> future_test(bool err)
{
    if (err == true)
    {
        return hsd::runtime_error{
            "Error from future_test(bool err = true)"
        };
    }

    return {};
}

hsd::result<hsd::i32, hsd::runtime_error> future_test(hsd::i32 i)
{
    if (i < 0)
    {
        return hsd::runtime_error{
            "Error from future_test(hsd::i32 i)"
        };
    }

    return i;
}

auto future_test(hsd::f32 f)
{
    struct future_test_result;

    using RefType = hsd::reference<future_test_result>;
    using RetType = hsd::result<RefType, hsd::runtime_error>;

    struct future_test_result
    {
        hsd::i32 i;
        hsd::f32 f;
    };
    
    static auto res = future_test_result {
        .i = static_cast<hsd::i32>(f),
        .f = f - static_cast<hsd::i32>(f)
    };

    if (f < 0.f)
    {
        return RetType {
            hsd::runtime_error {
                "Error from future_test(hsd::f32 f)"
            }
        };
    }

    return RetType{RefType{res}};
}

int main()
{
    {
        // future from a packaged_task
        hsd::packaged_task task = {[]{ return 7; }}; // wrap the function
        hsd::future f1 = task.get_future();  // get a future
        hsd::thread t = {hsd::move(task)}; // launch on a thread

        // future from a promise
        hsd::promise<hsd::i32, void> p;
        hsd::future f2 = p.get_future();
        hsd::thread{
            [&p]{ p.set_value(9); }
        }.detach().unwrap();

        f1.wait();
        f2.wait();
        assert(f1.get().unwrap() == 7);
        assert(f2.get().unwrap() == 9);

        t.join().unwrap();
    }

    {
        hsd::promise<hsd::i32, void> actual_promise;
        std::promise<hsd::i32> standard_promise;
        auto actual_future = actual_promise.get_future();
        auto standard_future = standard_promise.get_future();

        actual_promise.set_value(314);
        standard_promise.set_value(314);
        actual_future.wait();
        standard_future.wait();
        assert(actual_future.get().unwrap() == standard_future.get());
    }

    {
        auto (*f)(bool) = future_test;
        // future that will not error
        hsd::packaged_task task = {f};
        hsd::future f1 = task.get_future();
        hsd::thread t = {hsd::move(task), false};

        // future that will error
        hsd::packaged_task task2 = {f};
        hsd::future f2 = task2.get_future();
        hsd::thread t2 = {hsd::move(task2), true};

        f1.wait();
        f2.wait();
        assert(f1.get().is_ok());
        assert(!f2.get().is_ok());

        t.join().unwrap();
        t2.join().unwrap();
    }

    {
        auto (*f)(hsd::i32) = future_test;
        // future that will not error
        hsd::packaged_task task = {f};
        hsd::future f1 = task.get_future();
        hsd::thread t = {hsd::move(task), 1223};

        // future that will error
        hsd::packaged_task task2 = {f};
        hsd::future f2 = task2.get_future();
        hsd::thread t2 = {hsd::move(task2), -357};

        f1.wait();
        f2.wait();
        assert(f1.get().is_ok() && f1.get().unwrap() == 1223);
        assert(!f2.get().is_ok());

        t.join().unwrap();
        t2.join().unwrap();
    }

    {
        auto (*f)(hsd::f32) = future_test;
        // Initialize the shared reference result deterministically before starting either task.
        assert(f(1.223f).is_ok());

        // future that will not error
        hsd::packaged_task task = {f};
        hsd::future f1 = task.get_future();
        hsd::thread t = {hsd::move(task), 1.223f};

        // future that will error
        hsd::packaged_task task2 = {f};
        hsd::future f2 = task2.get_future();
        hsd::thread t2 = {hsd::move(task2), -3.57f};

        f1.wait();
        f2.wait();
        assert(f1.get().is_ok());
        assert(f1.get().unwrap().get().f > 0.22f);
        assert(!f2.get().is_ok());

        t.join().unwrap();
        t2.join().unwrap();
    }
}