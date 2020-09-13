#pragma once

#include "Utility.hpp"
#include "Tuple.hpp"

#include <type_traits> // std::decay_t
#include <pthread.h>
#include <cstdlib>

namespace hsd 
{
	struct thread 
	{
		struct id 
		{
			friend thread;
	
			id() = default;
	
			friend bool operator==(id l, id r) 
			{
				return l.id_ == r.id_;
			}
		private:
	
			u64 id_;
	
			id(pthread_t pt) 
				: id_{static_cast<u64>(pt)} 
			{}
		};
	
		using native_handle_type = pthread_t;
	
		thread() 
			:id_{0} 
		{}
	
		~thread() 
		{
			if (joinable())
				abort();
		}
	
		template <typename F, typename... Args>
		thread(F&& func, Args&&... args) 
		{
			pthread_attr_t attr;
			pthread_attr_init(&attr);
	
			auto decay_copy = []<typename T>(T&& t) -> std::decay_t<T>
			{
				return hsd::forward<T>(t);
			};
	
			hsd::i32 ready = 0;
	
			struct thread_data 
			{
				hsd::i32* ready;
	
				std::decay_t<F> func;
				hsd::tuple<std::decay_t<Args>...> args;
	
				static void* enter_thread(void* arg) 
				{
					auto td = hsd::move(*reinterpret_cast<thread_data*>(arg));
	
					// Tell our parent we are ready and copied the data
					__atomic_store_n(td.ready, 1, __ATOMIC_RELEASE);
					hsd::apply(td.func, td.args);
	
					return nullptr;
				}
			} td {
				&ready,
				decay_copy(hsd::forward<F>(func)),
				hsd::make_tuple(decay_copy(hsd::forward<Args>(args))...)
			};
	
			pthread_create(&id_, &attr, thread_data::enter_thread, &td);
	
			// Spinlock time
			while (!__atomic_load_n(&ready, __ATOMIC_ACQUIRE));
		}
	
		thread(const thread &) = delete;
		thread &operator=(const thread &) = delete;
	
		thread(thread &&other) 
			:id_{0} 
		{
			swap(other); 
		}
	
		thread &operator=(thread &&other) 
		{
			swap(other); return *this; 
		}
	
		void swap(thread &other) 
		{ 
			hsd::swap(id_, other.id_); 
		}
	
		id get_id() 
		{
			return id{id_};
		}
	
		bool joinable() {
			return get_id() != id{};
		}
	
		native_handle_type native_handle() 
		{
			return id_;
		}
	
		static u32 hardware_concurrency() 
		{
			return 0;
		}
	
		void detach() 
		{
			pthread_detach(id_);
			id_ = 0;
		}
	
		void join() 
		{
			pthread_join(id_, nullptr);
			id_ = 0;
		}
	
	private:
		pthread_t id_;
	}; // struct thread
} // namespace hsd
