// Title: Fibonacci with big integers
// Level: Advanced
// Shows: iterative and fast-doubling Fibonacci in integer<1024>, a constexpr check, and gcd.

#include <Containers/Integer.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

using big = hsd::integer<1024>;

// Works at compile time too: integer<N> operators are constexpr.
template <hsd::usize N>
constexpr hsd::integer<N> fib_iterative(hsd::u32 n)
{
    hsd::integer<N> previous = 0;
    hsd::integer<N> current = 1;

    for (hsd::u32 step = 0; step < n; ++step)
    {
        const hsd::integer<N> next = previous + current;
        previous = current;
        current = next;
    }

    return previous;
}

// F(90) is the largest Fibonacci number below 2^63, checked while compiling.
static_assert(fib_iterative<128>(90) == hsd::integer<128>{2'880'067'194'370'816'120ULL});
static_assert(fib_iterative<128>(10) == hsd::integer<128>{55});

struct pair_of_fibs
{
    big current;
    big following;
};

// Fast doubling: F(2k) = F(k) * (2 F(k+1) - F(k)) and F(2k+1) = F(k)^2 + F(k+1)^2.
// It needs about log2(n) steps, each using big multiplications.
static pair_of_fibs fib_doubling(hsd::u32 n)
{
    if (n == 0)
    {
        return {big{0}, big{1}};
    }

    const pair_of_fibs half = fib_doubling(n / 2);
    const big twice = half.current * (half.following * 2 - half.current);
    const big square_sum = half.current * half.current
        + half.following * half.following;

    if (n % 2 == 0)
    {
        return {twice, square_sum};
    }

    return {square_sum, twice + square_sum};
}

static big gcd(big lhs, big rhs)
{
    while (rhs.any())
    {
        const big remainder = lhs % rhs;
        lhs = rhs;
        rhs = remainder;
    }
    return lhs;
}

int main()
{
    const hsd::u32 samples[] = {10, 50, 100, 300, 1000};

    for (const hsd::u32 n : samples)
    {
        const big slow = fib_iterative<1024>(n);
        const big fast = fib_doubling(n).current;
        hsd::println("F({}) = {}"_fmt, n, slow);
        hsd::println("  both methods agree: {}"_fmt, slow == fast ? "true" : "false");
    }

    // Neighbouring Fibonacci numbers share no factor, while F(m) divides F(k*m).
    const big f1000 = fib_iterative<1024>(1000);
    const big f999 = fib_iterative<1024>(999);
    const big f500 = fib_iterative<1024>(500);
    const big f100 = fib_iterative<1024>(100);
    const big f25 = fib_iterative<1024>(25);
    hsd::println("gcd(F(1000), F(999)) = {}"_fmt, gcd(f1000, f999));
    hsd::println("F(500) divides F(1000): {}"_fmt, (f1000 % f500).none() ? "true" : "false");
    hsd::println("gcd(F(100), F(1000)) = F(100): {}"_fmt, gcd(f1000, f100) == f100 ? "true" : "false");
    hsd::println("gcd(F(100), F(25)) = {}"_fmt, gcd(f100, f25));

    return 0;
}
