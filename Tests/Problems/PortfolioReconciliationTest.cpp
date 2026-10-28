#include <cassert>

#include <Containers/UnorderedMap.hpp>
#include <Extra/Json.hpp>
#include <Serialize/StringParser.hpp>

using namespace hsd::string_view_literals;

struct position_state
{
    hsd::i32 quantity = 0;
    hsd::i64 cost_basis_cents = 0;
};

using positions = hsd::unordered_map<hsd::string, position_state>;

static auto apply_trade(
    positions& holdings,
    hsd::i64& cash_cents,
    hsd::i64& realized_pnl_cents,
    const char* line)
    -> hsd::option_err<hsd::runtime_error>
{
    auto parsed = hsd::parse_string<"{} {} {} {}", 4>(line);
    if (!parsed.is_ok())
    {
        return hsd::runtime_error{"Malformed trade record"};
    }

    auto fields = parsed.unwrap();
    const auto command = fields[0].to_string_view();
    hsd::string symbol = fields[1].to_string();
    const hsd::i32 quantity = fields[2].to_i32();
    const hsd::i64 unit_price = fields[3].to_i64();
    if (quantity <= 0 || unit_price < 0)
    {
        return hsd::runtime_error{"Invalid trade quantity or price"};
    }

    if (command == "BUY"_sv)
    {
        auto& position = holdings[symbol];
        position.quantity += quantity;
        position.cost_basis_cents += static_cast<hsd::i64>(quantity) * unit_price;
        cash_cents -= static_cast<hsd::i64>(quantity) * unit_price;
        return {};
    }

    if (command == "SELL"_sv)
    {
        auto existing = holdings.at(symbol);
        if (!existing.is_ok())
        {
            return hsd::runtime_error{"Cannot sell an unowned position"};
        }

        auto& position = existing.unwrap().get();
        if (quantity > position.quantity)
        {
            return hsd::runtime_error{"Sale exceeds the position quantity"};
        }

        const hsd::i64 removed_basis =
            position.cost_basis_cents * quantity / position.quantity;
        const hsd::i64 proceeds = static_cast<hsd::i64>(quantity) * unit_price;
        position.quantity -= quantity;
        position.cost_basis_cents -= removed_basis;
        cash_cents += proceeds;
        realized_pnl_cents += proceeds - removed_basis;
        return {};
    }

    return hsd::runtime_error{"Unknown trade command"};
}

int main()
{
    positions holdings;
    hsd::i64 cash_cents = 0;
    hsd::i64 realized_pnl_cents = 0;

    assert(apply_trade(holdings, cash_cents, realized_pnl_cents, "BUY ABC 10 1000").is_ok());
    assert(apply_trade(holdings, cash_cents, realized_pnl_cents, "BUY XYZ 4 2500").is_ok());
    assert(apply_trade(holdings, cash_cents, realized_pnl_cents, "SELL ABC 3 1200").is_ok());
    assert(!apply_trade(holdings, cash_cents, realized_pnl_cents, "SELL XYZ 5 3000").is_ok());
    assert(!apply_trade(holdings, cash_cents, realized_pnl_cents, "SELL MISSING 1 100").is_ok());

    assert(cash_cents == -16400);
    assert(realized_pnl_cents == 600);
    assert(holdings.at("ABC").unwrap().get().quantity == 7);
    assert(holdings.at("ABC").unwrap().get().cost_basis_cents == 7000);
    assert(holdings.at("XYZ").unwrap().get().quantity == 4);
    assert(holdings.at("XYZ").unwrap().get().cost_basis_cents == 10000);

    constexpr auto quote_data = R"json({"ABC": 1300, "XYZ": 2300})json"_sv;
    hsd::JsonStream stream;
    stream.lex(quote_data).unwrap();
    stream.push_eot().unwrap();
    hsd::JsonParser parser = stream;
    auto quotes = parser.parse_next().unwrap();

    hsd::i64 market_value_cents = 0;
    hsd::i64 unrealized_pnl_cents = 0;
    for (const auto& [symbol, position] : holdings)
    {
        const auto symbol_view = static_cast<hsd::string_view>(symbol);
        const hsd::i32 quote = (*quotes)[symbol_view].as_num<hsd::i32>().unwrap();
        market_value_cents += static_cast<hsd::i64>(position.quantity) * quote;
        unrealized_pnl_cents +=
            static_cast<hsd::i64>(position.quantity) * quote - position.cost_basis_cents;
    }

    assert(market_value_cents == 18300);
    assert(unrealized_pnl_cents == 1300);
    assert(cash_cents + market_value_cents == 1900);
    assert(realized_pnl_cents + unrealized_pnl_cents == 1900);
}