#include "../../cpp/Thread.hpp"
#include "../../cpp/Io.hpp"

static void foo(int a, float b) 
{
	hsd::io::print<"{} {}\n">(a, b);
}

int main() 
{
	hsd::thread t{foo, 1, 2.3f};
	t.join();
}
