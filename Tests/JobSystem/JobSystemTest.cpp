#define JOB_SYSTEM_COMP_UNIT
#include <JobSystem.hpp>

#include <Io.hpp>

void print()
{
    hsd::io::print<L"hello, and other words\n">();
}

int main()
{
    hsd::Job j = JobSys.createJob([](hsd::Job j) 
    {
        print();
    });

    for(int i = 0; i < 100; i++)
    {
        JobSys.scheduleJob(j);
    }
    
    JobSys.wait();
}