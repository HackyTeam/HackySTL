#pragma once

#include "Utility.hpp"
#include "Tuple.hpp"

#if defined(HSD_PLATFORM_POSIX)

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
	
			friend bool operator==(id lhs, id rhs) 
			{
				return lhs._id == rhs._id;
			}

		private:
			u64 _id;
	
			id(pthread_t pt) 
				: _id{static_cast<u64>(pt)} 
			{}
		};
	
		using native_handle_type = pthread_t;
	
		thread() 
			: _id{0} 
		{}
	
		~thread() 
		{
			if (joinable())
				abort();

			detach();
		}
	
		template <typename F, typename... Args>
		thread(F&& func, Args&&... args) 
		{
			pthread_attr_t attr;
			pthread_attr_init(&attr);
	
			auto decay_copy = []<typename T>(T&& t) -> decay_t<T>
			{
				return hsd::forward<T>(t);
			};
	
			hsd::i32 ready = 0;
	
			struct thread_data 
			{
				hsd::i32* ready;
	
				hsd::decay_t<F> func;
				hsd::tuple<decay_t<Args>...> args;
	
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
	
			pthread_create(&_id, &attr, thread_data::enter_thread, &td);
	
			// Spinlock time
			while (!__atomic_load_n(&ready, __ATOMIC_ACQUIRE));
		}
	
		thread(const thread&) = delete;
		thread& operator=(const thread&) = delete;
	
		thread(thread&& other) 
			:_id{0} 
		{
			swap(other); 
		}
	
		thread& operator=(thread&& other) 
		{
			swap(other); return *this; 
		}
	
		void swap(thread &other) 
		{ 
			hsd::swap(_id, other._id); 
		}
	
		id get_id() 
		{
			return id{_id};
		}
	
		bool joinable() {
			return get_id() != id{};
		}
	
		native_handle_type native_handle() 
		{
			return _id;
		}
	
		static u32 hardware_concurrency() 
		{
			return 0;
		}
	
		void detach() 
		{
			pthread_detach(_id);
			_id = 0;
		}
	
		void join() 
		{
			pthread_join(_id, nullptr);
		}
	
	private:
		pthread_t _id;
	}; // struct thread
} // namespace hsd
#else

#include <windows.h>

namespace hsd 
{
	class thread 
	{
	private:
		struct thread_data_base
		{
			virtual ~thread_data_base() {}
		};

		DWORD _id;
		HANDLE _handle;
		thread_data_base* _td;

	public:
		struct id 
		{
			friend thread;
	
			id() = default;
	
			friend bool operator==(id lhs, id rhs) 
			{
				return lhs._id == rhs._id;
			}
		private:
	
			u64 _id;
	
			id(DWORD pt) 
				: _id{static_cast<u64>(pt)} 
			{}
		};
	
		using native_handle_type = HANDLE;
	
		thread() 
			: _id{0}, _handle{nullptr}, _td{nullptr}
		{}
	
		~thread() 
		{
			if (joinable())
				abort();

			detach();
		}
	
		template <typename F, typename... Args>
		thread(F&& func, Args&&... args) 
		{
			auto decay_copy = []<typename T>(T&& t) -> decay_t<T>
			{
				return hsd::forward<T>(t);
			};
	
			struct thread_data 
				: public thread_data_base
			{
				hsd::decay_t<F> func;
				hsd::tuple<decay_t<Args>...> args;

				using func_type = hsd::decay_t<F>;
				using args_type = hsd::tuple<decay_t<Args>...>;
	
				thread_data(func_type&& _func, args_type&& _args)
					: func{forward<func_type>(_func)}, 
					args{forward<args_type>(_args)}
				{}

				~thread_data() = default;

				static DWORD enter_thread(void* arg) 
				{
					auto* td = reinterpret_cast<thread_data*>(arg);
					hsd::apply(td->func, td->args);
	
					return 0ul;
				}
			};
	
			_td = new thread_data{
				decay_copy(hsd::forward<F>(func)),
				hsd::make_tuple(decay_copy(hsd::forward<Args>(args))...)
			};

			_handle = CreateThread(
				nullptr, 0, thread_data::enter_thread, _td, 0, &_id
			); 
		}
	
		thread(const thread&) = delete;
		thread& operator=(const thread&) = delete;
	
		thread(thread&& other) 
			: _id{0}, _handle{nullptr}, _td{nullptr}
		{
			swap(other);
		}
	
		thread& operator=(thread&& other) 
		{
			swap(other); return *this; 
		}
	
		void swap(thread& other) 
		{ 
			hsd::swap(_id, other._id);
			hsd::swap(_td, other._td);
			hsd::swap(_handle, other._handle); 
		}
	
		id get_id() 
		{
			return id{_id};
		}
	
		bool joinable() {
			return get_id() != id{};
		}
	
		native_handle_type native_handle() 
		{
			return _handle;
		}
	
		static u32 hardware_concurrency() 
		{
			return 0;
		}
	
		void detach() 
		{
			CloseHandle(_handle);
			delete _td;

			_id = 0;
			_handle = nullptr;
			_td = nullptr;
		}
	
		void join() 
		{
			WaitForSingleObject(_handle, INFINITE);
			_id = 0;
		}
	}; // struct thread
} // namespace hsd
#endif