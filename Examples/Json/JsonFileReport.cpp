// Title: JSON file report
// Level: Advanced
// Shows: lex_file on a fixture, safe lookups with access(), and a summary computed from the tree.

#include <Extra/Json.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

int main()
{
    // inventory.json sits next to this program and is copied to the run directory.
    hsd::JsonStream lexer;
    auto lexed = lexer.lex_file("inventory.json");

    if (!lexed.is_ok())
    {
        hsd::println("could not read inventory.json"_fmt);
        return 0;
    }

    lexer.push_eot().unwrap();
    hsd::JsonParser parser = lexer;
    auto parsed = parser.parse_next();

    if (!parsed.is_ok())
    {
        hsd::println("invalid JSON"_fmt);
        return 0;
    }

    auto root = parsed.unwrap();
    hsd::println("warehouse: {}"_fmt, (*root)["warehouse"_sv].as_str().unwrap());

    hsd::usize active = 0;
    hsd::usize out_of_stock = 0;
    hsd::i32 total_units = 0;
    hsd::f32 total_value = 0.f;

    auto& items = (*root)["items"_sv];

    for (auto& item : items.as_array())
    {
        auto qty = (*item)["qty"_sv].as_num<hsd::i32>().unwrap();
        auto price = (*item)["price"_sv].as_num<hsd::f32>().unwrap();
        bool is_active = (*item)["active"_sv].as_bool().unwrap();

        // The "discount" key is absent from every item, so access() reports it as a miss
        // where operator[] would abort.
        if (item->access("discount"_sv).is_ok())
        {
            hsd::println("unexpected discount"_fmt);
        }

        if (is_active)
            active++;

        if (qty == 0)
            out_of_stock++;

        total_units += qty;
        total_value += static_cast<hsd::f32>(qty) * price;

        hsd::println("{} {} qty: {} value: {prec=2}"_fmt,
            (*item)["sku"_sv].as_str().unwrap(),
            (*item)["name"_sv].as_str().unwrap(),
            qty, static_cast<hsd::f32>(qty) * price);
    }

    hsd::println("items: {} active: {} out of stock: {}"_fmt,
        items.as_array().size(), active, out_of_stock);
    hsd::println("units: {} value: {prec=2}"_fmt, total_units, total_value);

    hsd::println("notes:"_fmt);

    for (auto& note : (*root)["notes"_sv].as_array())
    {
        hsd::println("- {}"_fmt, note->as_str().unwrap());
    }

    return 0;
}
