#include <cassert>

#include <Containers/Range.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Extra/Json.hpp>

using namespace hsd::string_view_literals;

struct replay_outcome
{
    hsd::string order_id;
    bool committed;
    hsd::i64 total_cents;
};

class inventory_ledger
{
    hsd::unordered_map<hsd::string, hsd::i32> _stock;
    hsd::unordered_map<hsd::string, hsd::i32> _unit_prices;
    hsd::unordered_map<hsd::string, bool> _processed_orders;
    hsd::i64 _revenue = 0;

public:
    void load_catalog(hsd::JsonValue& catalog)
    {
        for (auto& [sku, definition] : catalog.as_object())
        {
            _stock[sku] = (*definition)["quantity"_sv]
                .as_num<hsd::i32>().unwrap();
            _unit_prices[sku] = (*definition)["unit_cents"_sv]
                .as_num<hsd::i32>().unwrap();
        }
    }

    auto apply(hsd::JsonValue& order)
        -> hsd::result<hsd::i64, hsd::runtime_error>
    {
        hsd::string order_id{order["id"_sv].as_str().unwrap()};
        if (_processed_orders.at(order_id).is_ok())
        {
            return hsd::runtime_error{"Duplicate order ID"};
        }

        hsd::unordered_map<hsd::string, hsd::i32> requested_quantities;
        hsd::i64 order_total = 0;

        for (auto& line : order["lines"_sv].as_array())
        {
            hsd::string sku{(*line)["sku"_sv].as_str().unwrap()};
            const hsd::i32 quantity = (*line)["quantity"_sv]
                .as_num<hsd::i32>().unwrap();

            if (quantity <= 0)
            {
                return hsd::runtime_error{"Order quantity must be positive"};
            }

            auto price = _unit_prices.at(sku);
            if (!_stock.at(sku).is_ok() || !price.is_ok())
            {
                return hsd::runtime_error{"Unknown SKU"};
            }

            requested_quantities[sku] += quantity;
            order_total += static_cast<hsd::i64>(price.unwrap().get()) * quantity;
        }

        for (const auto& [sku, quantity] : requested_quantities)
        {
            if (quantity > _stock.at(sku).unwrap().get())
            {
                return hsd::runtime_error{"Insufficient stock"};
            }
        }

        for (const auto& [sku, quantity] : requested_quantities)
        {
            _stock.at(sku).unwrap().get() -= quantity;
        }

        _processed_orders[order_id] = true;
        _revenue += order_total;
        return order_total;
    }

    hsd::i32 stock_of(hsd::string_view sku) const
    {
        return _stock.at(hsd::string{sku}).unwrap().get();
    }

    hsd::i64 revenue() const
    {
        return _revenue;
    }
};

int main()
{
    constexpr auto document = R"json({
        "catalog": {
            "tea": {"quantity": 12, "unit_cents": 250},
            "coffee": {"quantity": 5, "unit_cents": 450},
            "cake": {"quantity": 2, "unit_cents": 700}
        },
        "orders": [
            {
                "id": "order-001",
                "lines": [
                    {"sku": "tea", "quantity": 3},
                    {"sku": "tea", "quantity": 2},
                    {"sku": "cake", "quantity": 1}
                ]
            },
            {
                "id": "order-002",
                "lines": [
                    {"sku": "coffee", "quantity": 2},
                    {"sku": "tea", "quantity": 4}
                ]
            },
            {
                "id": "order-003",
                "lines": [
                    {"sku": "coffee", "quantity": 1},
                    {"sku": "cake", "quantity": 2}
                ]
            },
            {
                "id": "order-001",
                "lines": [{"sku": "tea", "quantity": 1}]
            },
            {
                "id": "order-005",
                "lines": [{"sku": "oats", "quantity": 1}]
            }
        ]
    })json"_sv;

    hsd::JsonStream stream;
    const hsd::usize first_split = document.length() / 3;
    const hsd::usize second_split = document.length() * 2 / 3;
    stream.lex({document.data(), first_split}).unwrap();
    stream.lex({document.data() + first_split, second_split - first_split}).unwrap();
    stream.lex({document.data() + second_split, document.length() - second_split}).unwrap();
    stream.push_eot().unwrap();

    hsd::JsonParser parser = stream;
    auto root = parser.parse_next().unwrap();
    inventory_ledger ledger;
    ledger.load_catalog((*root)["catalog"_sv]);

    hsd::vector<replay_outcome> outcomes;
    for (auto& order : (*root)["orders"_sv].as_array())
    {
        hsd::string order_id{(*order)["id"_sv].as_str().unwrap()};
        auto result = ledger.apply(*order);
        const bool committed = result.is_ok();
        const hsd::i64 total = committed ? result.unwrap() : 0;
        outcomes.push_back({hsd::move(order_id), committed, total});
    }

    assert(outcomes.size() == 5);
    assert(outcomes[0].committed && outcomes[0].total_cents == 1950);
    assert(outcomes[1].committed && outcomes[1].total_cents == 1900);
    assert(!outcomes[2].committed);
    assert(!outcomes[3].committed);
    assert(!outcomes[4].committed);

    hsd::i64 filtered_revenue = 0;
    hsd::usize committed_count = 0;
    for (const auto& outcome : outcomes | hsd::ranges::filter(
        [](const replay_outcome& entry) { return entry.committed; }
    ))
    {
        filtered_revenue += outcome.total_cents;
        ++committed_count;
    }

    assert(committed_count == 2);
    assert(filtered_revenue == ledger.revenue());
    assert(ledger.revenue() == 3850);
    assert(ledger.stock_of("tea"_sv) == 3);
    assert(ledger.stock_of("coffee"_sv) == 3);
    assert(ledger.stock_of("cake"_sv) == 1);
}