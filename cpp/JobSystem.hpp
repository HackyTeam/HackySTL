#pragma once

#include "Thread.hpp"
#include "Functional.hpp"
#include "Vector.hpp"
#include "MPMCQueue.hpp"

#include <atomic>

namespace hsd
{
    struct Job;

    using JobFn = void(*)(Job);
    using Counter = std::atomic<usize>;

    // A Job. Has a function ptr and a pointer to data to pass as args
    struct Job
    {
        JobFn task;
        void** data = nullptr;

    };

/// @TODO: Change from using a void** to pass args (which is super bad and unsafe), to this JobData struct
/*
    // A packed list of arguments for a function in a job
    template<u8 numargs, typename T>
    struct JobData
    {
        
    };
*/

    enum class PRIO
    {
        HIGH = 0,
        NORM,
        LOW,

        NUM_PRIORITIES = 3
    };

    namespace priv
    {
        class JobSystem
        {
        private:
            hsd::Counter counter = {0};
            std::atomic_bool running = {false};

            MPMCQueue<Job> high_priority;
            MPMCQueue<Job> normal_priority;
            MPMCQueue<Job> low_priority;

            hsd::vector<hsd::thread> threads;

            void thread_function()
            {
                hsd::Job job;
                while(running.load())
                {
                    if(high_priority.try_pop(job))
                    {
                        job.task(job);
                    }
                    else if(normal_priority.try_pop(job))
                    {
                        job.task(job);
                    }
                    else if(low_priority.try_pop(job))
                    {
                        job.task(job);
                    }
                }
            }

        public:
            JobSystem() : high_priority(512), normal_priority(512), low_priority(512)
            {
                running.store(true);

                auto numcores = hsd::thread::hardware_concurrency(); // <- this function is not yet implemented, and always returns 0
                numcores = 4;   // Temporary // @TODO: implement ^ functions

                for(auto i = 0; i < numcores; i++)
                {
                    threads.emplace_back(hsd::thread(&JobSystem::thread_function, this));
                }
            }

            ~JobSystem()
            {
                running.store(false);

                for(auto& t : threads)
                {
                    t.join();
                }
            }

            //Returns the id of the caller from the threadpool index [0, max), and returns -1 if the caller isn't in the list
            i8 getCurrentThread() const
            {
                return -1; //hsd::thread::this_thread::id is not implemented as an equivalent to the std version,
                           //therefore, this currently cannot be implemented

                /*auto& id = hsd::thread::get_id();
                for(auto i = 0; i < threads.size(); i++)
                {
                    if(id == threads.at(i).get_id())
                    {
                        return i;
                    }
                }
                return -1;*/
            }
            // Returns a Job which may be scheduled.
            static Job createJob(hsd::JobFn jfunct, void** data = nullptr)
            {
                return Job{jfunct, data};
            }
            // Schedules a job to be added to the queue. Optionally accepts a priority for the job
            void scheduleJob(Job& j, hsd::PRIO p = hsd::PRIO::NORM)
            {
                switch(p)
                {
                    case hsd::PRIO::HIGH:
                        high_priority.emplace(hsd::forward<Job>(j));
                        counter++;
                        return;
                    case hsd::PRIO::NORM:
                        normal_priority.emplace(hsd::forward<Job>(j));
                        counter++;
                        return;
                    case hsd::PRIO::LOW:
                        low_priority.emplace(hsd::forward<Job>(j));
                        counter++;
                        return;
                }
            }
            // Wait until the number of remaining tasks is equal to or below the ``target``, the second argument will use the caller thread to execute jobs as well if set to ``true``
            void wait(int target = 0, bool useCurrentThread = true)
            {
                while(counter.load() > target)
                {
                    if(useCurrentThread)
                    {
                        Job job;
                        if(high_priority.try_pop(job))
                        {
                            job.task(job);
                            counter--;
                        }
                        else if(normal_priority.try_pop(job))
                        {
                            job.task(job);
                            counter--;
                        }
                        else if(low_priority.try_pop(job))
                        {
                            job.task(job);
                            counter--;
                        }

                    }
                }
            }
        
        };
    }

    // Global Job System. It is not recommended to make a new one, as the threads last for the lifetime of the program (global)
    static inline priv::JobSystem JobSys;
    
}
