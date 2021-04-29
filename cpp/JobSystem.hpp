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
    using Counter = std::atomic<size_t>;

    struct Job
    {
        JobFn task;
        void** data = nullptr;
    };

/*
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
                numcores = 4;   // Temporary

                for(auto i = 0; i < 4; i++)
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

            static Job createJob(hsd::JobFn jfunct, void** data = nullptr)
            {
                return Job{jfunct, data};
            }

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

    static inline priv::JobSystem JobSys;
    
}
