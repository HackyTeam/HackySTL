#pragma once

#include "Thread.hpp"
#include "MPMCQueue.hpp"
#include "Functional.hpp"

namespace hsd
{
    using job_fn = function<void()>;

    enum class PRIO
    {
        HIGH = 0,
        NORM,
        LOW
    };

    namespace priv
    {
        class JobSystem
        {
        private:
            counter _counter = {0};
            atomic_bool _running = {false};

            MPMCQueue<job_fn> _high_priority;
            MPMCQueue<job_fn> _normal_priority;
            MPMCQueue<job_fn> _low_priority;

            vector<thread> _threads;

            void thread_function()
            {
                job_fn _job_task = nullptr;

                while (_running.load())
                {
                    if (_high_priority.try_pop(_job_task))
                    {
                        _job_task().unwrap();
                        _counter--;
                    }
                    else if (_normal_priority.try_pop(_job_task))
                    {
                        _job_task().unwrap();
                        _counter--;
                    }
                    else if (_low_priority.try_pop(_job_task))
                    {
                        _job_task().unwrap();
                        _counter--;
                    }
                }
            }

        public:
            JobSystem()
                : _high_priority(512), 
                _normal_priority(512), 
                _low_priority(512)
            {
                _running.store(true);
                usize _numcores = thread::hardware_concurrency(); 

                for (usize _index = 0; _index < _numcores; _index++)
                {
                    _threads.emplace_back(bind(&JobSystem::thread_function, this));
                }
            }

            ~JobSystem()
            {
                _running.store(false);

                for (auto& _thread : _threads)
                {
                    _thread.join().unwrap();
                }
            }

            //Returns the id of the caller from the threadpool index [0, max), and returns -1 if the caller isn't in the threadpool
            isize get_current_thread() const
            {
                auto _this_thread_id = this_thread::get_id();

                for (usize _index = 0; _index < _threads.size(); _index++)
                {
                    if (_this_thread_id == _threads[_index].get_id())
                    {
                        return static_cast<isize>(_index);
                    }
                }

                return -1;
            }

            // Schedules a job to be added to the queue. Optionally accepts a priority for the job
            void schedule_job(job_fn job_task, PRIO priority = PRIO::NORM)
            {
                switch (priority)
                {
                    case PRIO::HIGH:
                        _high_priority.emplace(move(job_task));
                        _counter++;
                        return;
                    case PRIO::NORM:
                        _normal_priority.emplace(move(job_task));
                        _counter++;
                        return;
                    case PRIO::LOW:
                        _low_priority.emplace(move(job_task));
                        _counter++;
                        return;
                }
            }

            // Wait until the number of remaining tasks
            // is equal to or below the `target`, the
            // second argument will use the caller thread
            // to execute jobs as well if set to `true`
            void wait(ulong target = 0, bool use_current_thread = true)
            {
                while (_counter.load() > target)
                {
                    if (use_current_thread)
                    {
                        job_fn _job_task = nullptr;

                        if (_high_priority.try_pop(_job_task))
                        {
                            _job_task().unwrap();
                            _counter--;
                        }
                        else if (_normal_priority.try_pop(_job_task))
                        {
                            _job_task().unwrap();
                            _counter--;
                        }
                        else if (_low_priority.try_pop(_job_task))
                        {
                            _job_task().unwrap();
                            _counter--;
                        }
                    }
                }
            }
        
        };
    }

    // Global Job System. It is not recommended
    // to make a new one, as the threads last
    // for the lifetime of the program (global)
    static inline priv::JobSystem job_system;
}