#include <Thread.hpp>
#include <Future.hpp>
#include <Io.hpp>

hsd::Result<void, hsd::runtime_error> future_test(bool err)
{
    if (err == true)
    {
        return hsd::runtime_error{
            "Error from future_test(bool err = true)"
        };
    }

    return {};
}

hsd::Result<hsd::i32, hsd::runtime_error> future_test(hsd::i32 i)
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
    using RetType = hsd::Result<RefType, hsd::runtime_error>;

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
    using namespace hsd::format_literals;

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

        hsd::println("Waiting non-erroring void calls"_fmt);
        
        f1.wait();
        f2.wait();
        
        hsd::println(
            "Results are: {} {}"_fmt, 
            f1.get().unwrap(), f2.get().unwrap()
        );

        t.join().unwrap();
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

        hsd::println("Waiting erroring void calls"_fmt);
        
        f1.wait();
        f2.wait();
        
        hsd::println(
            "Results are: '{}' '{}'"_fmt, 
            f1.get().is_ok() ? "void" : f1.get().unwrap_err()(),
            f2.get().is_ok() ? "void" : f2.get().unwrap_err()()
        );

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

        hsd::println("Waiting erroring retruning calls"_fmt);
        
        f1.wait();
        f2.wait();
        
        hsd::println(
            "Results are: '{}' '{}'"_fmt, 
            f1.get().is_ok() ? f1.get().unwrap() : -1,
            f2.get().is_ok() ? f2.get().unwrap() : -1
        );

        t.join().unwrap();
        t2.join().unwrap();
    }

    {
        auto (*f)(hsd::f32) = future_test;
        // future that will not error
        hsd::packaged_task task = {f};
        hsd::future f1 = task.get_future();
        hsd::thread t = {hsd::move(task), 1.223f};

        // future that will error
        hsd::packaged_task task2 = {f};
        hsd::future f2 = task2.get_future();
        hsd::thread t2 = {hsd::move(task2), -3.57f};

        hsd::println("Waiting erroring retruning calls"_fmt);
        
        f1.wait();
        f2.wait();
        
        hsd::println(
            "Results are: '{}' '{}'"_fmt, 
            f1.get().is_ok() ? f1.get().unwrap().f : -1.f,
            f2.get().is_ok() ? f2.get().unwrap().f : -1.f
        );

        t.join().unwrap();
        t2.join().unwrap();
    }
}