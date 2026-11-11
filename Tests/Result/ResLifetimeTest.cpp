#include <cassert>

#include <Base/Result.hpp>
#include <Support/TestSupport.hpp>

// Copies, moves, assignments and destruction of result, option and option_err, in both
// states, with a value type that counts its live objects. Every test ends with no
// objects alive, so a missed destructor or a double destruction fails the check.
namespace
{
    struct counted
    {
        static inline int live = 0;
        int value = 0;

        counted()
        {
            ++live;
        }

        explicit counted(int new_value)
            : value{new_value}
        {
            ++live;
        }

        counted(const counted& other)
            : value{other.value}
        {
            ++live;
        }

        counted(counted&& other) noexcept
            : value{other.value}
        {
            other.value = -1;
            ++live;
        }

        counted& operator=(const counted& other)
        {
            value = other.value;
            return *this;
        }

        counted& operator=(counted&& other) noexcept
        {
            value = other.value;
            other.value = -1;
            return *this;
        }

        ~counted()
        {
            --live;
        }
    };

    struct failure
    {
        const char* message;

        const char* pretty_error() const
        {
            return message;
        }
    };

    using counted_result = hsd::result<counted, failure>;
    using counted_option = hsd::option<counted>;
    using counted_option_err = hsd::option_err<failure>;

    counted_result make_ok(int value)
    {
        return counted{value};
    }

    counted_result make_err(const char* message)
    {
        return failure{message};
    }

    counted_option make_option(int value)
    {
        return counted{value};
    }

    counted_option make_empty_option()
    {
        return hsd::err_value{};
    }

    counted_option_err make_option_err(const char* message)
    {
        return failure{message};
    }

    counted_option_err make_empty_option_err()
    {
        return hsd::ok_value{};
    }

    void check_result_moves()
    {
        // Move construction and assignment across both states.
        counted_result _ok = make_ok(1);
        counted_result _err = make_err("first");

        counted_result _moved_ok{hsd::move(_ok)};
        counted_result _moved_err{hsd::move(_err)};
        assert(_moved_ok.is_ok() && _moved_ok.unwrap().value == 1);
        assert(!_moved_err.is_ok());
        assert(_moved_err.unwrap_err().message[0] == 'f');

        // Assignment from an error into a success and from a success into an error.
        _moved_ok = hsd::move(_moved_err);
        assert(!_moved_ok.is_ok());
        _moved_err = make_ok(2);
        assert(_moved_err.is_ok());

        // Assignment between two successes and between two errors.
        counted_result _second_ok = make_ok(3);
        _moved_err = hsd::move(_second_ok);
        assert(_moved_err.is_ok());

        counted_result _second_err = make_err("second");
        _moved_ok = hsd::move(_second_err);
        assert(!_moved_ok.is_ok());
    }

    void check_result_copies()
    {
        // Copies made from const references go through the copy constructors.
        const counted _value{4};
        const failure _failure{"copy"};

        counted_result _ok{_value};
        counted_result _err{_failure};
        assert(_ok.is_ok() && _ok.unwrap().value == 4);
        assert(!_err.is_ok());

        // unwrap_or and unwrap_or_else return a fallback in the error state.
        counted_result _fallback = make_err("fallback");
        assert(_fallback.unwrap_or(counted{9}).value == 9);
        assert(make_ok(5).unwrap_or_default().value == 5);
        assert(make_err("default").unwrap_or_default().value == 0);
        assert(make_err("lambda").unwrap_or_else([]() { return counted{11}; }).value == 11);
        assert(make_ok(6).unwrap_or_else([]() { return counted{12}; }).value == 6);

        // The accessors that expect the other state return that state's data.
        assert(make_err("error").unwrap_err().message[0] == 'e');
        assert(make_err("expected").expect_err("expected an error").message[0] == 'e');
        assert(make_ok(7).expect("expected a value").value == 7);

        // The bool conversion reports success.
        assert(static_cast<bool>(make_ok(8)));
        assert(!static_cast<bool>(make_err("no")));
    }

    void check_option_lifetimes()
    {
        const counted _value{13};
        counted_option _copied{_value};
        counted_option _moved{make_option(14)};
        counted_option _empty = make_empty_option();

        assert(_copied.is_ok() && _copied.unwrap().value == 13);
        assert(_moved.is_ok() && _moved.unwrap().value == 14);
        assert(!_empty.is_ok());

        // Move between states, then move-construct from both.
        _empty = hsd::move(_moved);
        assert(_empty.is_ok());
        _moved = make_empty_option();
        assert(!_moved.is_ok());
        _moved = make_option(15);
        counted_option _from_move{hsd::move(_moved)};
        assert(_from_move.unwrap_or(counted{0}).value == 15);

        // Fallbacks and the empty-option accessors.
        assert(make_empty_option().unwrap_or(counted{16}).value == 16);
        assert(make_empty_option().unwrap_or_default().value == 0);
        assert(make_empty_option().unwrap_or_else([]() { return counted{17}; }).value == 17);
        make_empty_option().unwrap_err();
        make_empty_option().expect_err("expected an empty option");
        assert(static_cast<bool>(make_option(18)) && !static_cast<bool>(make_empty_option()));
    }

    void check_option_err_lifetimes()
    {
        counted_option_err _error = make_option_err("error");
        counted_option_err _success = make_empty_option_err();
        assert(!_error.is_ok() && _success.is_ok());

        counted_option_err _moved{hsd::move(_error)};
        assert(!_moved.is_ok());

        // Move assignment in both directions.
        _success = hsd::move(_moved);
        assert(!_success.is_ok());
        _moved = make_empty_option_err();
        assert(_moved.is_ok());

        // Accessors on the matching state succeed; the others have their own tests.
        assert(make_option_err("message").unwrap_err().message[0] == 'm');
        assert(make_option_err("expect").expect_err("expected an error").message[0] == 'e');
        make_empty_option_err().unwrap();
        make_empty_option_err().expect("expected success");
        assert(static_cast<bool>(make_empty_option_err()) && !static_cast<bool>(make_option_err("x")));
    }
}

int main()
{
    check_result_moves();
    check_result_copies();
    check_option_lifetimes();
    check_option_err_lifetimes();

    // Every object created above has been destroyed.
    assert(counted::live == 0);
}
