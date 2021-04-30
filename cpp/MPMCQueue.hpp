#pragma once

#include <new>

#include "Vector.hpp"
#include "Atomic.hpp"


namespace hsd
{
    
    using counter = hsd::atomic_usize;

    namespace priv
    {
        template<typename T>
        struct slot
        {


            void destroy()
            {
                storage.~T();
            }

            ~slot()
            {
                destroy();
            }
            hsd::atomic_usize ticket = {0};

            T storage;
        };
    }

    /**
     * Multiprocessor Multiconsumer Queue [MPMCQueue]
     * Written by PyroFlareX, inspiration from github.com/rigtorp/MPMCQueue for concept.
     * This is a "thread-safe" MPMC queue written for HackySTL, type and max capacity must be known on creation.
     * Probably not the best optimized implementation, but it works well enough.
     * Works as a stack-machine, FIFO
    */
    template<typename T>
    class MPMCQueue
    {
    public:
        MPMCQueue(usize capacity)  :   _capacity(capacity)
        {
            if(_capacity < 1)
            {
                //This is UB
                puts("Capacity is less than 1, this is UB, and an error\n");
                abort();
            }
            _allocation.resize(_capacity + 1);
        }

        MPMCQueue(const MPMCQueue&) = delete;
        MPMCQueue& operator=(const MPMCQueue&) = delete;

        bool try_emplace(T&& v)
        {
            auto head = _head.load();

            for( ; ;)
            {
                auto& slot = _allocation.at_unchecked(mod(head));

                if(turn(head) * 2 == slot.ticket.load())
                {
                    if(_head.compare_exchange_strong(head, head + 1))
                    {
                        slot.storage = hsd::forward<T>(v);

                        slot.ticket.store(turn(head) * 2 + 1);

                        return true;
                    }
                }
                else
                {
                    auto const prevHead = head;
                    head = _head.load();

                    if(head == prevHead)
                    {
                        return false;
                    }
                }
            }

            return false;
        }
        bool try_pop(T& v)
        {
            auto tail = _tail.load();

            for( ; ; )
            {
                auto& slot = _allocation.at_unchecked(mod(tail));
                if(turn(tail) * 2 + 1 ==  slot.ticket.load())
                {
                    if(_tail.compare_exchange_strong(tail, tail + 1))
                    {
                        v = hsd::move(slot.storage);

                        slot.destroy();
                        slot.ticket.store(turn(tail) * 2 + 2);
                        return true;
                    }
                }
                else
                {
                    auto const prevTail = tail;
                    tail = _tail.load();
                    if(tail == prevTail)
                    {
                        return false;
                    }
                }
            }
        }

        void emplace(T&& v)
        {
            auto const head = _head.fetch_add(1);

            auto& slot = _allocation.at_unchecked(mod(head));

            while(turn(head) * 2 != slot.ticket.load())
            {
                //wait until condition is satisfied
            }
            
            slot.storage = hsd::forward<T>(v);

            slot.ticket.store(turn(head) * 2 + 1);
        }
        void pop(T& v)
        {
            auto const tail = _tail.fetch_add(1);
            
            auto& slot = _allocation.at_unchecked(mod(tail));

            while(turn(tail) * 2 + 1 != slot.ticket.load())
            {
                //wait until condition is satisfied
            }

            v = hsd::move(slot.storage);
            slot.destroy();

            slot.ticket.store(turn(tail) * 2 + 2);
        }

    private:
        usize _capacity;

        vector<priv::Slot<T>> _allocation;

        hsd::atomic_usize _head = {0};
        hsd::atomic_usize _tail = {0};

        constexpr size_t mod(size_t i) const noexcept { return i % capacity; }
        constexpr size_t turn(size_t i) const noexcept { return i / capacity; }
    };
}