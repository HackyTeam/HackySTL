#include <Threading/Thread.hpp>
#include <Threading/Atomic.hpp>
#include <Wrappers/Functional.hpp>
#include <cassert>
#include <thread>

hsd::atomic_i32 calls{};

class bar
{
private:
	int _a, _b;

public:
	constexpr bar(int a, int b)
		: _a{a}, _b{b}
	{}

	void foo() 
	{
		calls.fetch_add(_a + _b, hsd::memory_order_relaxed);
	}
};

int main() 
{
	hsd::thread t{hsd::bind(&bar::foo, bar{211, 3223})};
	assert(t.joinable());
	assert(!(t.get_id() == hsd::this_thread::get_id()));

	hsd::thread transferred{hsd::move(t)};
	assert(!t.joinable());
	assert(transferred.joinable());
	transferred.join().unwrap();
	assert(!transferred.joinable());
	assert(!transferred.join().is_ok());
	assert(calls.load() == 3434);

	hsd::thread assignment_source{hsd::bind(&bar::foo, bar{1, 2})};
	hsd::thread assignment_target;
	assignment_target = hsd::move(assignment_source);
	assert(!assignment_source.joinable());
	assignment_target.join().unwrap();
	assert(calls.load() == 3437);

	int standard_calls = 0;
	std::thread standard_source([&] { standard_calls += 3434; });
	assert(standard_source.joinable());
	std::thread standard_transferred{std::move(standard_source)};
	assert(!standard_source.joinable());
	assert(standard_transferred.joinable());
	standard_transferred.join();
	assert(!standard_transferred.joinable());
	assert(standard_calls == calls.load() - 3);

	std::thread standard_assignment_source([&] { standard_calls += 3; });
	std::thread standard_assignment_target;
	standard_assignment_target = std::move(standard_assignment_source);
	assert(!standard_assignment_source.joinable());
	standard_assignment_target.join();
	assert(standard_calls == calls.load());
}
