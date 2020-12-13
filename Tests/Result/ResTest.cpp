#include "../../cpp/Result.hpp"
#include "../../cpp/String.hpp"

struct S
{
    const char* operator()() const
    {
        return "Test";
    }
};

static auto fail_func(int v) -> hsd::Result<hsd::string, S>
{
    if(v < 10)
        return S{};
    else
        return {"Test string"};
}

template <typename T>
static constexpr auto fail_func2(const T& v) -> hsd::Result<T, S>
{
    if(!static_cast<bool>(v))
        return S{};
    else
        return v;
}

int main()
{
    [](const auto& param)
    {
        auto v = fail_func2(param);
        v.unwrap(HSD_FUNCION_NAME);
        v.expect_err("fuck you error", HSD_FUNCION_NAME);
    }(10);

    fail_func(10).unwrap();
    //fail_func(1).unwrap_err();
    fail_func(10).expect("Test");
    //fail_func(1).expect_err("Test");

    fail_func(10).unwrap_or();
    fail_func(10).unwrap_or_default();
    fail_func(10).unwrap_or_else(
        []() -> hsd::string {
            puts("Fail, here comes a new value");
            return "New string";
        }
    );
}
