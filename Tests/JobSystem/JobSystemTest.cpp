#include <JobSystem.hpp>

#include <Io.hpp>

void print()
{
    hsd::io::print<L"hello, and other words\n">();
}

// Example function that takes args
void foo(int a, float b)
{
    auto z = a / b;

    z = (a & (a + 1)) + b - z/a;
    
    hsd::io::print<L"Result: {}">(z);

    return;
}

int main()
{
    // Generic example with no parameters
    hsd::Job j = hsd::JobSys.createJob([](hsd::Job j) 
    {
        print();
    });

    // Schedules 100 jobs
    for(int i = 0; i < 100; i++)
    {
        hsd::JobSys.scheduleJob(j);
    }
    
    // Pauses current function of the caller until all jobs are done
    hsd::JobSys.wait();

//                              \\//

    //Example where args are passed
    int x = 10;
    float y = 5.942f;

    void* data[] = { &x, &y };

    j = hsd::JobSys.createJob([](hsd::Job j)    // Technically could just be a fnptr, but a lambda is more readable here
    {
        // Get the variables from the data array
        auto& s = *static_cast<int*>(j.data[0]);    //Can be mutable
        auto t = *static_cast<float*>(j.data[1]);   // Or a copy

        // Passing the values into the function
        foo(s, t);

    }, data);
}