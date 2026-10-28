// Title: Sieve of Eratosthenes
// Level: Intermediate
// Shows: a large bitset as a compact flag table, driven by a limit read from stdin.

#include <Containers/Bitset.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

constexpr hsd::usize max_limit = 10'000;

// Bit i is set when i is composite. Starting all clear means "assume prime".
// One bit per number keeps the table at about a kilobyte for 10000 numbers.
static hsd::usize sieve(hsd::bitset<max_limit + 1>& composite, hsd::usize limit)
{
    composite.reset();

    for (hsd::usize number = 2; number * number <= limit; ++number)
    {
        if (composite[number])
        {
            continue;
        }

        for (hsd::usize multiple = number * number; multiple <= limit; multiple += number)
        {
            composite[multiple] = true;
        }
    }

    hsd::usize primes = 0;
    for (hsd::usize number = 2; number <= limit; ++number)
    {
        if (composite[number] == false)
        {
            ++primes;
        }
    }

    return primes;
}

int main()
{
    hsd::bitset<max_limit + 1> composite;

    // One limit per line. The stream hands over a value once more input
    // follows it, so the input finishes with a 0 that also stops the loop.
    for (auto limit = hsd::io::cin().read_value<hsd::usize>(); limit.is_ok();
        limit = hsd::io::cin().read_value<hsd::usize>())
    {
        const hsd::usize bound = limit.unwrap();

        if (bound == 0)
        {
            break;
        }

        if (bound < 2 || bound > max_limit)
        {
            hsd::println("limit {} is out of range"_fmt, bound);
            continue;
        }

        const hsd::usize primes = sieve(composite, bound);
        hsd::println("primes up to {}: {}"_fmt, bound, primes);

        // Show the smallest ones, then the twin pairs below the bound.
        hsd::print("first:"_fmt);
        hsd::usize shown = 0;
        for (hsd::usize number = 2; number <= bound && shown < 8; ++number)
        {
            if (composite[number] == false)
            {
                hsd::print(" {}"_fmt, number);
                ++shown;
            }
        }
        hsd::println(""_fmt);

        hsd::usize twins = 0;
        for (hsd::usize number = 3; number + 2 <= bound; ++number)
        {
            if (composite[number] == false && composite[number + 2] == false)
            {
                ++twins;
            }
        }
        hsd::println("twin prime pairs: {}"_fmt, twins);
    }

    return 0;
}
