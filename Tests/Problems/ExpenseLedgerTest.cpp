#include <cassert>
#include <string>
#include <unordered_map>

#include <Containers/Range.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/StringParser.hpp>

#include <Support/TestSupport.hpp>

using ledger = hsd::unordered_map<hsd::string, int>;

static void apply_record(ledger& accounts, const char* record)
{
    auto fields = hsd::parse_string<"{} {} {}", 3>(record).unwrap();
    const auto command = fields[0].to_string();
    const auto name = fields[1].to_string();
    const auto amount = fields[2].to_i32();

    assert(amount >= 0);

    if (test_support::to_string(command) == "deposit")
    {
        accounts[name] += amount;
    }
    else if (test_support::to_string(command) == "withdraw")
    {
        assert(accounts[name] >= amount);
        accounts[name] -= amount;
    }
    else
    {
        assert(false);
    }
}

static void apply_transfer(ledger& accounts, const char* record)
{
    auto fields = hsd::parse_string<"{} {} {} {}", 4>(record).unwrap();
    const auto command = fields[0].to_string();
    const auto source = fields[1].to_string();
    const auto destination = fields[2].to_string();
    const auto amount = fields[3].to_i32();

    assert(test_support::to_string(command) == "transfer");
    assert(amount >= 0);
    assert(accounts[source] >= amount);
    accounts[source] -= amount;
    accounts[destination] += amount;
}

static void test_ledger()
{
    ledger accounts;
    const char* deposits[] = {
        "deposit alice 120",
        "deposit bob 40",
        "deposit alice 30",
        "withdraw bob 7"
    };

    for (const auto* record : deposits)
    {
        apply_record(accounts, record);
    }

    apply_transfer(accounts, "transfer alice bob 50");
    apply_transfer(accounts, "transfer bob alice 8");

    assert(accounts["alice"] == 108);
    assert(accounts["bob"] == 75);
    assert(accounts.size() == 2);

    hsd::vector<int> balances;
    for (const auto& [name, balance] : accounts)
    {
        (void)name;
        balances.push_back(balance);
    }

    int positive_total = 0;
    for (const auto balance : balances | hsd::ranges::filter(
        [](int value) { return value > 0; }
    ))
    {
        positive_total += balance;
    }
    assert(positive_total == 183);

    std::unordered_map<std::string, int> expected{
        {"alice", 108}, {"bob", 75}
    };
    assert(accounts.size() == expected.size());
    for (const auto& [name, balance] : expected)
    {
        assert(accounts.at(name.c_str()).unwrap().get() == balance);
    }
}

int main()
{
    test_ledger();
}