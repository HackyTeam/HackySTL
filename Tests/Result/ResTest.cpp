#include <cassert>
#include <expected>
#include <optional>
#include <string>

#include <Base/Result.hpp>
#include <Containers/String.hpp>
#include <Support/TestSupport.hpp>

struct test_error
{
    const char* message;

    const char* pretty_error() const
    {
        return message;
    }
};

static hsd::result<hsd::string, test_error> parse(bool succeeds)
{
    if (succeeds)
    {
        return hsd::string{"parsed"};
    }
    return test_error{"parse failed"};
}

static void test_result_success()
{
    auto result = parse(true);
    std::expected<std::string, std::string> standard_result{
        std::in_place, "parsed"
    };
    assert(result.is_ok());
    assert(result.is_ok() == standard_result.has_value());
    assert(static_cast<bool>(result));
    const auto actual_text = test_support::to_string(result.unwrap());
    assert(actual_text == "parsed");
    assert(actual_text == *standard_result);

    assert(test_support::to_string(parse(true).unwrap_or("fallback")) == "parsed");
    assert(test_support::to_string(parse(true).unwrap_or("fallback"))
        == standard_result.value_or("fallback"));
    assert(test_support::to_string(parse(true).unwrap_or_default()) == "parsed");
    assert(test_support::to_string(
        parse(true).unwrap_or_else([] { return hsd::string{"fallback"}; })
    ) == "parsed");
}

static void test_result_error()
{
    auto result = parse(false);
    std::expected<std::string, std::string> standard_result{
        std::unexpected{std::string{"parse failed"}}
    };
    assert(!result.is_ok());
    assert(result.is_ok() == standard_result.has_value());
    assert(!static_cast<bool>(result));
    assert(std::string{result.unwrap_err().pretty_error()} == "parse failed");
    assert(std::string{result.unwrap_err().pretty_error()} == standard_result.error());
    assert(test_support::to_string(parse(false).unwrap_or("fallback")) == "fallback");
    assert(test_support::to_string(parse(false).unwrap_or("fallback"))
        == standard_result.value_or("fallback"));
    assert(parse(false).unwrap_or_default().length() == 0);
    assert(test_support::to_string(
        parse(false).unwrap_or_else([] { return hsd::string{"recovered"}; })
    ) == "recovered");
}

static void test_option_types()
{
    hsd::option<int> value{12};
    std::optional<int> standard_value{12};
    assert(value.is_ok());
    assert(value.is_ok() == standard_value.has_value());
    assert(value.unwrap() == 12);
    assert(value.unwrap() == *standard_value);

    hsd::option<int> missing{hsd::err_value{}};
    std::optional<int> standard_missing;
    assert(!missing.is_ok());
    assert(missing.is_ok() == standard_missing.has_value());
    assert(missing.unwrap_or(33) == 33);
    assert(missing.unwrap_or(33) == standard_missing.value_or(33));
    assert(missing.unwrap_or_else([] { return 44; }) == 44);

    hsd::option_err<test_error> error{test_error{"bad"}};
    assert(!error.is_ok());
    assert(std::string{error.unwrap_err().pretty_error()} == "bad");

    hsd::option<int> movable{12};
    std::optional<int> standard_movable{12};
    hsd::option<int> moved{hsd::move(movable)};
    std::optional<int> standard_moved{std::move(standard_movable)};
    assert(moved.is_ok());
    assert(moved.unwrap() == 12);
    assert(moved.unwrap() == *standard_moved);
}

int main()
{
    test_result_success();
    test_result_error();
    test_option_types();
}
