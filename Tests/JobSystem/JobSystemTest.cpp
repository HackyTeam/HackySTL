#include <JobSystem.hpp>

#include <Io.hpp>

void print()
{
    hsd::io::print<L"hello, and other words {}\n">(
        hsd::job_system.get_current_thread()
    );
}

// Example function that takes args
void foo(int a, float b)
{
    auto z = a / b;

    z = (a & (a + 1)) + b - z/a;
    
    hsd::io::print<L"Result: {}\n">(z);

    return;
}

int main()
{
    // Generic example with no parameters
    hsd::Job j = hsd::job_system.create_job([](hsd::Job) 
    {
        print();
    });

    // Schedules 10 jobs
    for(hsd::i32 i = 0; i < 10; i++)
    {
        hsd::job_system.schedule_job(j);
    }
    
    // Pauses current function of the caller until all jobs are done
    hsd::job_system.wait();

//                              \\//
    //Example where args are passed
    hsd::i32 x = 10;
    hsd::f32 y = 5.942f;

    void* data[] = { &x, &y };

    j = hsd::job_system.create_job([](hsd::Job j)    // Technically could just be a fnptr, but a lambda is more readable here
    {
        // Get the variables from the data array
        auto& s = *static_cast<hsd::i32*>(j.data[0]);    //Can be mutable
        auto t = *static_cast<hsd::f32*>(j.data[1]);   // Or a copy

        // Passing the values into the function
        foo(s, t);

    }, data);

    for(hsd::i32 i = 0; i < 10; ++i)
    {
        hsd::job_system.schedule_job(j);
    }

    hsd::job_system.wait();
}