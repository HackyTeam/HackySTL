#include <Result.hpp>
#include <String.hpp>

struct S
{
    const char* operator()() const
    {
        return "Catched an error";
    }
};

static auto fail_func(int val) 
    -> hsd::Result<hsd::string, S>
{
    if(val < 10)
        return S{};
    else
        return {"Test string"};
}

template <typename T>
static constexpr auto fail_func2(const T& val) 
    -> hsd::Result<hsd::string, S>
{
    if(!static_cast<bool>(val))
        return S{};
    else
        return hsd::to_string(val);
}

static hsd::optional<int> opt_fail()
{
    return hsd::bad_optional_access{};
}

int main()
{
    [](const auto& param)
    {
        auto v = fail_func2(param);
        //opt_fail().unwrap(HSD_FUNCTION_NAME);
        //v.unwrap_err(HSD_FUNCTION_NAME);
        v.expect("Goodbye error", HSD_FUNCTION_NAME);
    }(10);

    fail_func(10).unwrap();
    fail_func(1).unwrap_err();
    
    auto err1 = fail_func(1);
    puts(err1.is_ok() ? 
        err1.expect("Test").c_str() : 
        err1.expect_err("Test2")()
    );

    fail_func(10).expect("Test");
    
    auto err2 = fail_func2(1);
    puts(err2.is_ok() ? 
        err2.expect("Test").c_str() : 
        err2.expect_err("Test2")()
    );

    fail_func(10).unwrap_or("this");
    fail_func(10).unwrap_or_default();
    fail_func(1).unwrap_or_else(
        []() -> hsd::string {
            puts("Fail, here comes a new value");
            return "New string";
        }
    );
}
