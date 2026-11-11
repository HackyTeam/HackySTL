#include <cassert>

#include <Base/Result.hpp>
#include <Support/AbortSupport.hpp>
#include <Support/TestSupport.hpp>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

// Each unwrap-style accessor that fails ends the process with abort(). These checks run
// every failing path in a child process. Errors with pretty_error() print their message,
// and other error types print the expected and actual type names.
namespace
{
    struct described_error
    {
        const char* message;

        const char* pretty_error() const
        {
            return message;
        }
    };
}

int main()
{
    // Direct calls to the reporting helpers.
    test_support::expect_abort([] { hsd::panic("direct panic"); });
    test_support::expect_abort([] { hsd::panic_type_err<int, float>(); });
    test_support::expect_abort([] { hsd::unimplemented(); });

#if defined(__unix__) || defined(__APPLE__)
    // The diagnostic cannot be written when stderr is closed. panic still ends the process.
    test_support::expect_abort([]
    {
        ::close(2);
        hsd::panic("diagnostic written to a closed stderr");
    });
#endif

    // result: unwrap on an error with and without pretty_error().
    test_support::expect_abort([]
    {
        hsd::result<int, described_error> _res{described_error{"unwrap on error"}};
        (void)_res.unwrap();
    });
    test_support::expect_abort([]
    {
        hsd::result<int, float> _res{1.5f};
        (void)_res.unwrap();
    });

    // result: expect on an error reports the message it was given.
    test_support::expect_abort([]
    {
        hsd::result<int, described_error> _res{described_error{"expect on error"}};
        (void)_res.expect("expected a value");
    });

    // result: unwrap_err on a success, and expect_err on a success.
    test_support::expect_abort([]
    {
        hsd::result<int, described_error> _res{7};
        (void)_res.unwrap_err();
    });
    test_support::expect_abort([]
    {
        hsd::result<int, described_error> _res{7};
        (void)_res.expect_err("expected an error");
    });

    // option: unwrap and expect on an empty option, unwrap_err and expect_err on a value.
    test_support::expect_abort([]
    {
        hsd::option<int> _opt{hsd::err_value{}};
        (void)_opt.unwrap();
    });
    test_support::expect_abort([]
    {
        hsd::option<int> _opt{hsd::err_value{}};
        (void)_opt.expect("expected a value");
    });
    test_support::expect_abort([]
    {
        hsd::option<int> _opt{3};
        _opt.unwrap_err();
    });
    test_support::expect_abort([]
    {
        hsd::option<int> _opt{3};
        _opt.expect_err("expected an empty option");
    });

    // option_err: unwrap on an error with and without pretty_error(), and the accessors
    // that expect a value or an error on the wrong side.
    test_support::expect_abort([]
    {
        hsd::option_err<described_error> _opt{described_error{"option_err unwrap"}};
        _opt.unwrap();
    });
    test_support::expect_abort([]
    {
        hsd::option_err<float> _opt{1.5f};
        _opt.unwrap();
    });
    test_support::expect_abort([]
    {
        hsd::option_err<described_error> _opt{described_error{"option_err expect"}};
        _opt.expect("expected success");
    });
    test_support::expect_abort([]
    {
        hsd::option_err<described_error> _opt{hsd::ok_value{}};
        (void)_opt.unwrap_err();
    });
    test_support::expect_abort([]
    {
        hsd::option_err<described_error> _opt{hsd::ok_value{}};
        (void)_opt.expect_err("expected an error");
    });
}
