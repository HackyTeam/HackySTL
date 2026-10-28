// Title: Custom error types
// Level: Advanced
// Shows: errors with pretty_error, result of reference<>, and error propagation through option_err.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Reference.hpp>

using namespace hsd::format_literals;

enum class bank_code
{
    no_such_account,
    insufficient_funds,
    same_account
};

// Any type with a `const char* pretty_error() const` is understood by unwrap()
// when it panics, so a custom error only needs that one member.
struct bank_error
{
    bank_code code;

    const char* pretty_error() const
    {
        switch (code)
        {
            case bank_code::no_such_account: return "no such account";
            case bank_code::insufficient_funds: return "insufficient funds";
            default: return "cannot transfer to the same account";
        }
    }
};

struct account
{
    hsd::string name;
    hsd::i32 balance;
};

class bank
{
private:
    hsd::vector<account> _accounts;

public:
    void open(const char* name, hsd::i32 balance)
    {
        _accounts.push_back(account{hsd::string{name}, balance});
    }

    // A reference<> lets a result hand out a live element instead of a copy,
    // which a plain result<account&, ...> could not do.
    hsd::result<hsd::reference<account>, bank_error> find(const char* name)
    {
        hsd::string wanted{name};

        for (hsd::usize index = 0; index < _accounts.size(); ++index)
        {
            if (_accounts[index].name == wanted)
            {
                return hsd::reference<account>{_accounts[index]};
            }
        }

        return bank_error{bank_code::no_such_account};
    }

    hsd::option_err<bank_error> withdraw(const char* name, hsd::i32 amount)
    {
        auto found = find(name);
        if (!found.is_ok())
        {
            return found.unwrap_err();
        }

        account& target = found.unwrap().get();
        if (target.balance < amount)
        {
            return bank_error{bank_code::insufficient_funds};
        }

        target.balance -= amount;
        return {};
    }

    // Each step forwards its error unchanged, so the caller sees the first failure.
    hsd::option_err<bank_error> transfer(const char* from, const char* to, hsd::i32 amount)
    {
        if (hsd::string{from} == hsd::string{to})
        {
            return bank_error{bank_code::same_account};
        }

        auto destination = find(to);
        if (!destination.is_ok())
        {
            return destination.unwrap_err();
        }

        auto withdrawn = withdraw(from, amount);
        if (!withdrawn.is_ok())
        {
            return withdrawn.unwrap_err();
        }

        destination.unwrap().get().balance += amount;
        return {};
    }

    void print_balances() const
    {
        for (hsd::usize index = 0; index < _accounts.size(); ++index)
        {
            hsd::println("  {}: {}"_fmt, _accounts[index].name.c_str(), _accounts[index].balance);
        }
    }
};

static void report(const char* label, hsd::option_err<bank_error> outcome)
{
    if (outcome.is_ok())
    {
        hsd::println("{}: ok"_fmt, label);
    }
    else
    {
        hsd::println("{}: {}"_fmt, label, outcome.unwrap_err().pretty_error());
    }
}

int main()
{
    bank accounts;
    accounts.open("alice", 100);
    accounts.open("bob", 20);

    report("alice -> bob 30", accounts.transfer("alice", "bob", 30));
    report("bob -> alice 500", accounts.transfer("bob", "alice", 500));
    report("alice -> carol 5", accounts.transfer("alice", "carol", 5));
    report("bob -> bob 1", accounts.transfer("bob", "bob", 1));

    hsd::println("balances:"_fmt);
    accounts.print_balances();

    // The reference points into the bank, so writing through it changes the account.
    auto bob = accounts.find("bob");
    if (bob.is_ok())
    {
        bob.unwrap().get().balance += 1000;
    }
    hsd::println("after bonus:"_fmt);
    accounts.print_balances();

    // A reference<> in a failed result is never built, so the error is all there is.
    auto missing = accounts.find("dave");
    hsd::println("dave found: {}"_fmt, missing.is_ok() ? "true" : "false");

    return 0;
}
