#include <iostream>
#include "../../cpp/Thread.hpp"

void foo(int a, float b) {
	std::cout << a << " " << b << std::endl;
}

int main() {
	hsd::thread t{foo, 1, 2.3f};
	t.join();
}
