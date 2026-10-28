#include <Extra/Constexpr/Gorbitsa.hpp>
#include <cassert>
#include <print>

template <typename Str>
consteval auto run() 
{
	return hsd::run_parser<Str, true, 0, 0>();
}

int main()
{
	// Hello
	static constexpr char source[] = "S72 T S101 T S108 T T S111 T";

	// A 10x10 Box (By Megarev) (errors on gcc, bc it almost exceeds the stack size)
	//static constexpr char source[] = "S10 O0 G0 I255 O0 B19 S10 T S10 O1 G1 I255 O1 S35 T G1 B2 S0 B10 S10 T";

	// Simple stairs (By Megarev)
    //static constexpr char source[] = "S10 O10 S0 O1 S6 O0 G0 I255 O0 B24 t10 G1 I1 O1 O2 G2 I255 O2 S35 T G2 B6 S0 B15 t10";

	constexpr auto output = run< typename hsd::str_to_arr<source>::type >();
	static_assert(output.size() == 5);
	static_assert(output[0] == 'H');
	static_assert(output[1] == 'e');
	static_assert(output[2] == 'l');
	static_assert(output[3] == 'l');
	static_assert(output[4] == 'o');

	assert(output[0] == 'H');

	for (auto c : output)
	{
		std::print("{}", c);
	}

	std::println();
}