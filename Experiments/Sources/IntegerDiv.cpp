#include <Containers/Integer.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>

#if defined(_MSC_VER)
#define DIV_BENCH_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define DIV_BENCH_NOINLINE __attribute__((noinline))
#else
#define DIV_BENCH_NOINLINE
#endif

template <hsd::usize N>
struct quotient_remainder
{
    hsd::integer<N> quotient;
    hsd::integer<N> remainder;
};

template <hsd::usize N>
static DIV_BENCH_NOINLINE quotient_remainder<N> bitwise_divide(
    const hsd::integer<N>& numerator,
    const hsd::integer<N>& divisor)
{
    quotient_remainder<N> result{};

    for (hsd::usize bit = N; bit > 0; --bit)
    {
        result.remainder <<= 1;
        result.quotient <<= 1;
        result.remainder.set(0, numerator.test(bit - 1).unwrap()).unwrap();

        if (result.remainder >= divisor)
        {
            result.remainder -= divisor;
            result.quotient.set(0).unwrap();
        }
    }

    return result;
}

template <hsd::usize N>
static DIV_BENCH_NOINLINE quotient_remainder<N> word_divide(
    const hsd::integer<N>& numerator,
    const hsd::integer<N>& divisor)
{
    auto result = numerator.divide_with_reminder(divisor).unwrap();
    return {
        hsd::move(result.quotient),
        hsd::move(result.reminder)
    };
}

template <hsd::usize N>
static std::array<hsd::integer<N>, 8> make_inputs()
{
    std::array<hsd::integer<N>, 8> inputs{};
    std::uint64_t state = 0x9E37'79B9'7F4A'7C15ULL;

    for (hsd::usize sample = 0; sample < inputs.size(); ++sample)
    {
        for (hsd::usize bit = 0; bit < N; ++bit)
        {
            state ^= state << 13;
            state ^= state >> 7;
            state ^= state << 17;
            if ((state >> 63) != 0)
            {
                inputs[sample].set(bit).unwrap();
            }
        }
    }

    return inputs;
}

template <hsd::usize N>
using divider = quotient_remainder<N>(*)(
    const hsd::integer<N>&, const hsd::integer<N>&);

template <hsd::usize N>
static double measure(
    const std::array<hsd::integer<N>, 8>& inputs,
    const hsd::integer<N>& divisor,
    divider<N> divide,
    hsd::usize repetitions,
    std::uint64_t& checksum)
{
    const auto start = std::chrono::steady_clock::now();
    for (hsd::usize repetition = 0; repetition < repetitions; ++repetition)
    {
        for (const auto& input : inputs)
        {
            const auto result = divide(input, divisor);
            checksum += result.quotient.get_last_byte();
            checksum += result.remainder.get_last_byte();
        }
    }
    const auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::micro>(stop - start).count();
}

template <hsd::usize N>
static void benchmark_width(hsd::usize repetitions)
{
    const auto inputs = make_inputs<N>();
    const hsd::integer<N> divisor{0xFFFF'FFFBULL};

    for (const auto& input : inputs)
    {
        const auto old_result = bitwise_divide(input, divisor);
        const auto new_result = word_divide(input, divisor);
        assert(old_result.quotient == new_result.quotient);
        assert(old_result.remainder == new_result.remainder);
    }

    constexpr hsd::usize sample_count = 5;
    double old_times[sample_count]{};
    double new_times[sample_count]{};
    std::uint64_t old_checksum = 0;
    std::uint64_t new_checksum = 0;

    for (hsd::usize sample = 0; sample < sample_count; ++sample)
    {
        if ((sample & 1) == 0)
        {
            old_times[sample] = measure(
                inputs, divisor, bitwise_divide<N>, repetitions, old_checksum);
            new_times[sample] = measure(
                inputs, divisor, word_divide<N>, repetitions, new_checksum);
        }
        else
        {
            new_times[sample] = measure(
                inputs, divisor, word_divide<N>, repetitions, new_checksum);
            old_times[sample] = measure(
                inputs, divisor, bitwise_divide<N>, repetitions, old_checksum);
        }
    }

    assert(old_checksum == new_checksum);
    std::sort(old_times, old_times + sample_count);
    std::sort(new_times, new_times + sample_count);
    const double old_median = old_times[sample_count / 2];
    const double new_median = new_times[sample_count / 2];

    std::printf(
        "%4zu-bit (%zu divisions/sample): bitwise %9.2f us, "
        "word-wise %9.2f us, speedup %7.2fx\n",
        N, repetitions * inputs.size(), old_median, new_median,
        old_median / new_median
    );
}

int main()
{
    benchmark_width<64>(3000);
    benchmark_width<256>(500);
    benchmark_width<1024>(125);
    benchmark_width<4096>(25);
    benchmark_width<8192>(10);
    benchmark_width<16384>(5);
}