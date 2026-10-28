#include <Extra/Constexpr/Gorbitsa.hpp>

// Regression tests for the memory-writing instructions O, o and r, checked at
// compile time. Without the fix these programs do not compile.
namespace
{
    template <const char* source>
    consteval auto run()
    {
        return hsd::run_parser<typename hsd::str_to_arr<source>::type, true, 0, 0>();
    }

    static constexpr char store_then_load[] = "S65 O3 R G3 T";
    static constexpr char reset_accumulator[] = "S65 R T";
    static constexpr char store_indirect[] = "S66 O4 S4 O5 G5 T";
    static constexpr char hello[] = "S72 T S101 T S108 T T S111 T";
}

static_assert(run<store_then_load>().size() == 1);
static_assert(run<store_then_load>()[0] == 65);
static_assert(run<reset_accumulator>()[0] == 0);
static_assert(run<store_indirect>().size() == 1);
static_assert(run<hello>().size() == 5);
static_assert(run<hello>()[0] == 'H');

int main()
{
}
