#pragma once

#include "../Utility/Utility.hpp"
#include "../Tuple/Tuple.hpp"
#include <type_traits> // std::decay_t
#include <pthread.h>
#include <cstdlib>

namespace hsd {

struct thread {
	struct id {
		friend thread;

		id() = default;

		friend bool operator==(id l, id r) {
			return l.id_ == r.id_;
		}
	private:
		id(pthread_t pt) :id_{reinterpret_cast<uint64_t>(pt)} { }
		uint64_t id_;
	};

	using native_handle_type = pthread_t;

	thread() :id_{0} {}

	~thread() {
		if (joinable())
			abort();
	}

	template <typename F, typename ...Args>
	thread(F &&func, Args &&...args) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		auto decay_copy = []<typename T>(T &&t) -> std::decay_t<T> {
			return hsd::forward<T>(t);
		};

		int ready = 0;

		struct thread_data {
			int *ready;

			std::decay_t<F> func;
			hsd::tuple<std::decay_t<Args>...> args;

			static void *enter_thread(void *arg) {
				// hsd::move is broken! this fails with:
				// error: invalid static_cast from 'const thread_data' to 'thread_data &&'
				auto td = std::move(*reinterpret_cast<thread_data *>(arg));

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
		while (!__atomic_load_n(&ready, __ATOMIC_ACQUIRE))
			;
	}

	thread(const thread &) = delete;
	thread &operator=(const thread &) = delete;

	thread(thread &&other) :id_{0} { swap(other); }
	thread &operator=(thread &&other) { swap(other); return *this; }

	void swap(thread &other) { hsd::swap(id_, other.id_); }

	id get_id() {
		return id{id_};
	}

	bool joinable() {
		return get_id() != id{};
	}

	native_handle_type native_handle() {
		return id_;
	}

	static unsigned int hardware_concurrency() {
		return 0;
	}

	void detach() {
		pthread_detach(id_);
		id_ = 0;
	}

	void join() {
		pthread_join(id_, nullptr);
		id_ = 0;
	}

private:
	pthread_t id_;
}; // struct thread

} // namespace hsd
