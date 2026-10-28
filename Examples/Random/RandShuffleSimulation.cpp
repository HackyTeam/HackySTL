// Title: Shuffle simulation
// Level: Advanced
// Shows: a Fisher-Yates shuffle on a seeded engine, and statistics gathered over many shuffles.

#include <Base/Random.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename Engine>
void shuffle(hsd::vector<hsd::u32>& cards, Engine& gen)
{
    // Walk from the back and swap each card with a random one at or before it.
    for (hsd::usize last = cards.size() - 1; last > 0; --last)
    {
        hsd::usize pick = gen.generate(hsd::usize{0}, last + 1).unwrap();
        hsd::u32 held = cards[last];
        cards[last] = cards[pick];
        cards[pick] = held;
    }
}

static hsd::vector<hsd::u32> make_deck(hsd::u32 size)
{
    hsd::vector<hsd::u32> deck;
    for (hsd::u32 card = 0; card < size; ++card)
    {
        deck.push_back(card);
    }
    return deck;
}

static void print_deck(const char* label, const hsd::vector<hsd::u32>& deck)
{
    hsd::print("{}:"_fmt, label);
    for (hsd::usize index = 0; index < deck.size(); ++index)
    {
        hsd::print(" {}"_fmt, deck[index]);
    }
    hsd::println(""_fmt);
}

int main()
{
    hsd::mt19937_64 gen{2024};

    auto deck = make_deck(10);
    shuffle(deck, gen);
    print_deck("shuffled", deck);

    // A shuffle must keep every card exactly once.
    bool seen[10]{};
    bool permutation = true;
    for (hsd::usize index = 0; index < deck.size(); ++index)
    {
        permutation = permutation && !seen[deck[index]];
        seen[deck[index]] = true;
    }
    hsd::println("still a permutation: {}"_fmt, permutation ? "true" : "false");

    // Replaying the same seed reproduces the very same shuffle.
    hsd::mt19937_64 replay{2024};
    auto copy = make_deck(10);
    shuffle(copy, replay);
    bool identical = true;
    for (hsd::usize index = 0; index < deck.size(); ++index)
    {
        identical = identical && deck[index] == copy[index];
    }
    hsd::println("replay matches: {}"_fmt, identical ? "true" : "false");

    // Simulate many shuffles of five cards: where does card 0 land, and how
    // often does a shuffle leave some card in its place?
    const hsd::u32 trials = 60000;
    hsd::u32 landing[5]{};
    hsd::u32 with_fixed_point = 0;

    for (hsd::u32 trial = 0; trial < trials; ++trial)
    {
        auto cards = make_deck(5);
        shuffle(cards, gen);

        bool fixed = false;
        for (hsd::u32 slot = 0; slot < 5; ++slot)
        {
            if (cards[slot] == 0)
            {
                ++landing[slot];
            }
            fixed = fixed || cards[slot] == slot;
        }
        with_fixed_point += fixed ? 1 : 0;
    }

    hsd::u32 total = 0;
    bool balanced = true;
    for (hsd::u32 slot = 0; slot < 5; ++slot)
    {
        total += landing[slot];
        // A fair shuffle puts card 0 in each slot about 12000 times.
        balanced = balanced && landing[slot] > 11000 && landing[slot] < 13000;
    }

    hsd::println("card 0 always lands somewhere: {}"_fmt, total == trials ? "true" : "false");
    hsd::println("landing slots look uniform: {}"_fmt, balanced ? "true" : "false");

    // The chance that a random shuffle of 5 has a fixed point is 76/120.
    hsd::f64 share = static_cast<hsd::f64>(with_fixed_point) / trials;
    hsd::println("fixed point share near 0.63: {}"_fmt,
        (share > 0.61 && share < 0.65) ? "true" : "false");

    return 0;
}
