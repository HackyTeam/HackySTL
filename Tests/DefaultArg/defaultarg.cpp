#include <Extra/DefaultArg.hpp>
#include <Io.hpp>

int add(hsd::i32 a, hsd::i32 b) 
{
    return a + b;
}

// see: "extern_impl.cpp"
extern "C" hsd::i32 bop(hsd::i32 a, hsd::i32 b) noexcept;
extern "C" hsd::function<hsd::i32()> f;

//static auto bopper = make_defaultcall<int(int, int)>(bop, 3, 4);
//static auto funcer = make_defaultcall<int()>(f);
hsd::defaultcall_t adder(&add, 3, 4);
static auto bopper = hsd::defaultcall_t(bop, 3, 4);
static auto funcer = hsd::defaultcall_t(hsd::forward<hsd::function<hsd::i32()>>(f));

int main() 
{
    using namespace hsd::format_literals;

    hsd::println("{}"_fmt, bopper(hsd::default_v, hsd::default_v));
    hsd::println("{}"_fmt, bopper(hsd::default_v, 1));
    hsd::println("{}"_fmt, bopper(2, hsd::default_v));
    hsd::println("{}"_fmt, bopper(2, 1));

    hsd::println("{}"_fmt, adder(5, hsd::default_v));

    hsd::println("{}"_fmt, funcer());
}