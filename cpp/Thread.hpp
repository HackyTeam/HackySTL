#pragma once

#include "Utility.hpp"
#include "Tuple.hpp"
#include "Result.hpp"

#if defined(HSD_PLATFORM_POSIX)
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <time.h>
#include <errno.h>
#include <string.h>
#elif defined(HSD_PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace hsd 
{
	#if defined(HSD_PLATFORM_POSIX)
	using native_id_type = u64;
	#else
	using native_id_type = DWORD;
	#endif
	
	struct id 
	{	
		id() = default;

		id(native_id_type id) 
			: _id{id}
		{}

		friend bool operator==(id lhs, id rhs) 
		{
			return lhs._id == rhs._id;
		}

	private:
		native_id_type _id;
	};

	struct this_thread
	{
		static id get_id() 
		{
			#if defined(HSD_PLATFORM_POSIX)
			return id{
				static_cast<native_id_type>(pthread_self())
			};
			#else
			return id{
				static_cast<native_id_type>(GetCurrentThreadId())
			};
			#endif
		}

		static Result<void, runtime_error> sleep_for(f32 seconds)
		{
			#if defined(HSD_PLATFORM_WINDOWS)
			    Sleep(seconds * 1000); // milliseconds
			#else /* posix */
    		auto frac = seconds - static_cast<i32>(seconds);
    		timespec ts{
    		    .tv_sec = static_cast<i32>(seconds),
    		    .tv_nsec = static_cast<long>(frac * 1'000'000'000.f)
    		};

    		while (nanosleep(&ts, &ts) == -1)
			{
        		if (errno != EINTR)
					return runtime_error{strerror(errno)};
			}
			#endif

			return {};
		}
	};

	struct thread 
	{
		#if defined(HSD_PLATFORM_POSIX)
		using native_handle_type = pthread_t;
		#else
		using native_handle_type = HANDLE;
		#endif

		thread() = default;

		~thread() 
		{
			if (joinable())
				abort();
		}

		template <typename F, typename... Args>
		thread(F&& func, Args&&... args) 
		{
			#if defined(HSD_PLATFORM_POSIX)
			pthread_attr_t _attr;
			pthread_attr_init(&_attr);
			#endif

			auto decay_copy = []<typename T>(T&& t) -> decay_t<T>
			{
				return hsd::forward<T>(t);
			};

			#if defined(HSD_PLATFORM_POSIX)
			hsd::i32 _ready = 0;
			#else
			hsd::u64 _ready = 0;
			#endif

			struct thread_data 
			{
				#if defined(HSD_PLATFORM_POSIX)
				hsd::i32* ready;
				#else
				hsd::u64* ready;
				#endif

				hsd::decay_t<F> func;
				hsd::tuple<decay_t<Args>...> args;

				#if defined(HSD_PLATFORM_POSIX)
				static void* enter_thread(void* arg)
				#else
				static DWORD enter_thread(void* arg)
				#endif
				{
					auto td = hsd::move(*reinterpret_cast<thread_data*>(arg));

					// Tell our parent we are ready and copied the data
					#if defined(HSD_PLATFORM_POSIX)
					__atomic_store_n(td.ready, 1, __ATOMIC_RELEASE);
					#elif defined(HSD_PLATFORM_WINDOWS)
					InterlockedIncrementRelease(td.ready);
					#endif
					
					hsd::apply(td.func, td.args);

					#if defined(HSD_PLATFORM_POSIX)
					return nullptr;
					#else
					return 0ul;
					#endif
				}
			}; 
			
			auto _td = thread_data
			{
				.ready = &_ready,
				.func = decay_copy(hsd::forward<F>(func)),
				.args = hsd::make_tuple(decay_copy(hsd::forward<Args>(args))...)
			};

			#if defined(HSD_PLATFORM_POSIX)
			i32 _res = pthread_create(&_handle, &_attr, thread_data::enter_thread, &_td);
			
			if (_res) abort();

			// Spinlock time
			while (!__atomic_load_n(&_ready, __ATOMIC_ACQUIRE));
			_id = id{static_cast<native_id_type>(_handle)};
			#else
			native_id_type _native_id;
			_handle = CreateThread(nullptr, 0, thread_data::enter_thread, &_td, 0, &_native_id);
			
			if (!_handle) abort();

			while (!InterlockedCompareExchangeAcquire(&_ready, 0, 1));
			_id = id{static_cast<native_id_type>(_native_id)};
			#endif
		}

		thread(const thread&) = delete;
		thread& operator=(const thread&) = delete;

		thread(thread&& other) 
		{
			swap(other); 
		}

		thread& operator=(thread&& other) 
		{
			swap(other); return *this;
		}

		void swap(thread &other) 
		{ 
			hsd::swap(_handle, other._handle);
			hsd::swap(_id, other._id);
		}

		id get_id() const
		{
			return id{_id};
		}

		bool joinable() 
		{
			return _id != id{};
		}

		native_handle_type native_handle() 
		{
			return _handle;
		}

		static auto hardware_concurrency() 
		{
			#if defined(HSD_PLATFORM_WINDOWS)
		    SYSTEM_INFO info;
		    GetSystemInfo(&info);
		    return info.dwNumberOfProcessors;
			#else
		    return sysconf(_SC_NPROCESSORS_ONLN);
			#endif
		}

		void detach() 
		{
			#if defined(HSD_PLATFORM_POSIX)
			pthread_detach(_handle);
			_handle = 0;
			#else
			CloseHandle(_handle);
			_handle = nullptr;
			#endif
			
			_id = {};
		}

		void join() 
		{
			#if defined(HSD_PLATFORM_POSIX)
			pthread_join(_handle, nullptr);
			_handle = 0;
			#else
			WaitForSingleObject(_handle, INFINITE);
			CloseHandle(_handle);
			_handle = nullptr;
			#endif

			_id = {};
		}
	
	private:
		native_handle_type _handle{};
		id _id{};
	}; // struct thread
} // namespace hsd