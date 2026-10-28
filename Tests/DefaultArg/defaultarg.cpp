#include <Extra/DefaultArg.hpp>
#include <cassert>

int add(hsd::i32 a, hsd::i32 b) 
{
    return a + b;
}

// see: "extern_impl.cpp"
extern "C" hsd::i32 bop(hsd::i32 a, hsd::i32 b) noexcept;
extern "C" hsd::function<hsd::i32()> f;

//static auto bopper = make_defaultcall<int(int, int)>(bop, 3, 4);
//static auto funcer = make_defaultcall<int()>(f);
hsd::default_call_t adder(&add, 3, 4);
static auto bopper = hsd::default_call_t(&bop, 3, 4);

int main() 
{
    auto funcer = hsd::default_call_t(
        hsd::forward<hsd::function<hsd::i32()>>(f)
    );

    assert(bopper(hsd::default_v, hsd::default_v) == 12);
    assert(bopper(hsd::default_v, 1) == 3);
    assert(bopper(2, hsd::default_v) == 8);
    assert(bopper(2, 1) == 2);
    assert(adder(5, hsd::default_v) == 9);
    assert(funcer() == 4);
}