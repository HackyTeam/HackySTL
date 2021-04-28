#pragma once

#include <atomic>
#include <new>

#include "Vector.hpp"


namespace hsd
{
    namespace priv
    {
        template<typename T>
        struct Slot
        {
            void destroy()
            {
                reinterpret_cast<T*>(&storage)->~T();
            }

            ~Slot()
            {
                destroy();
            }

            std::atomic<size_t> ticket = {0};

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
        MPMCQueue() = delete;
        MPMCQueue(size_t capacity)  :   capacity(capacity)
        {
            if(capacity < 1)
            {
                throw std::invalid_argument("MPMC capacity is less than 1");
            }
            allocation.resize(capacity + 1);
        }

        MPMCQueue(const MPMCQueue&) = delete;
        MPMCQueue& operator=(const MPMCQueue&) = delete;

        bool try_emplace(T&& v)
        {
            
        }
        bool try_pop(T& v)
        {
            auto tail = m_tail.load();

            for( ; ; )
            {
                auto& slot = allocation.at_unchecked(mod(tail));
                if(turn(tail) * 2 + 1 ==  slot.ticket.load())
                {
                    if(m_tail.compare_exchange_strong(tail, tail + 1))
                    {
                        v = std::move(slot);

                        slot.destroy();
                        slot.ticket.store(turn(tail) * 2 + 2);
                        return true;
                    }
                }
                else
                {
                    auto const prevTail = tail;
                    tail = m_tail.load();
                    if(tail == prevTail)
                    {
                        return false;
                    }
                }
            }
        }

        void emplace(T&& v)
        {
            auto const head = m_head.fetch_add(1);

            auto& slot = allocation.at_unchecked(mod(head));

            while(turn(head) * 2 != slot.ticket.load())
            {
                //wait until condition is satisfied
            }
            
            slot.storage = v;

            slot.ticket.store(turn(head) * 2 + 1);
        }
        void pop(T& v)
        {
            auto const tail = m_tail.fetch_add(1);
            
            auto& slot = allocation.at_unchecked(mod(tail));

            while(turn(tail) * 2 + 1 != slot.ticket.load())
            {
                //wait until condition is satisfied
            }

            v = std::move(slot.storage);
            slot.destroy();

            slot.ticket.store(turn(tail) * 2 + 2);
        }

    private:
        size_t capacity;

        vector<priv::Slot<T>> allocation;

        std::atomic<size_t> m_head = {0};
        std::atomic<size_t> m_tail = {0};

        constexpr size_t mod(size_t i) const noexcept { return i % capacity; }
        constexpr size_t turn(size_t i) const noexcept { return i / capacity; }
    };
}