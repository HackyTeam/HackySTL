// Title: Overriding some defaults
// Level: Intermediate
// Shows: default_call_t over a lambda, a make_defaultcall signature, and defaults in any position.

#include <Extra/DefaultArg.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Final price in cents: a base price, a tax percentage and a discount in cents.
hsd::i32 final_price(hsd::i32 base, hsd::i32 tax_percent, hsd::i32 discount)
{
    return (base - discount) * (100 + tax_percent) / 100;
}

int main()
{
    // Fallbacks: a base price of 1000 cents, 20 percent tax, and no discount.
    hsd::default_call_t price(&final_price, 1000, 20, 0);

    hsd::println("all defaults: {}"_fmt,
        price(hsd::default_v, hsd::default_v, hsd::default_v));

    // Only the discount is overridden. Any position may be left to its default.
    hsd::println("discount 200: {}"_fmt, price(hsd::default_v, hsd::default_v, 200));
    hsd::println("zero tax: {}"_fmt, price(hsd::default_v, 0, hsd::default_v));
    hsd::println("base 500, tax and discount default: {}"_fmt,
        price(500, hsd::default_v, hsd::default_v));
    hsd::println("nothing default: {}"_fmt, price(500, 10, 50));

    // A lambda works as well. Its call signature supplies the parameter types.
    hsd::default_call_t volume(
        [](hsd::i32 width, hsd::i32 height, hsd::i32 depth) { return width * height * depth; },
        2, 3, 4
    );
    hsd::println("volume: {}"_fmt, volume(hsd::default_v, hsd::default_v, hsd::default_v));
    hsd::println("taller volume: {}"_fmt, volume(hsd::default_v, 10, hsd::default_v));

    // make_defaultcall takes the call signature as a template argument instead
    // of deducing it from the callable.
    auto power = hsd::make_defaultcall<hsd::i32(hsd::i32, hsd::i32)>(
        [](hsd::i32 base, hsd::i32 exponent)
        {
            hsd::i32 result = 1;

            for (hsd::i32 step = 0; step < exponent; step++)
            {
                result *= base;
            }

            return result;
        },
        2, 8
    );
    hsd::println("2^8: {}"_fmt, power(hsd::default_v, hsd::default_v));
    hsd::println("3^8: {}"_fmt, power(3, hsd::default_v));
    hsd::println("2^10: {}"_fmt, power(hsd::default_v, 10));

    return 0;
}
