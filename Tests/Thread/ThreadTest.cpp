#include <Thread.hpp>
#include <Functional.hpp>
#include <Io.hpp>

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
		using namespace hsd::format_literals;

		hsd::println("{} {}"_fmt, _a, _b);
	}
};

int main() 
{
	hsd::thread t{hsd::bind(&bar::foo, bar{211, 3223})};
	t.join().unwrap();
}
