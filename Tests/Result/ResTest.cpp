#include "../../cpp/Result.hpp"

struct S
{
    const char* operator()() const
    {
        return "Test";
    }
};

static auto fail_func(int v) -> hsd::Result<void, S>
{
    if(v < 10)
        return S{};
    else
        return {};
}

int main()
{
    //fail_func(1).unwrap();
    fail_func(1).unwrap_err();
    //fail_func(1).expect("Test");
    fail_func(1).expect_err("Test");

    //fail_func(1).unwrap_or();
    //fail_func(1).unwrap_or_default();
    fail_func(1).unwrap_or_else([]{
        puts("Fail, here comes a new value");
    });
}
