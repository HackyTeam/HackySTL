// Title: Default arguments in callbacks
// Level: Advanced
// Shows: default_call_t wrapped in hsd::function, a pipeline of stages, and constexpr use.

#include <Containers/Vector.hpp>
#include <Extra/DefaultArg.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

hsd::i32 clamp_value(hsd::i32 value, hsd::i32 low, hsd::i32 high)
{
    return value < low ? low : (value > high ? high : value);
}

hsd::i32 scale_value(hsd::i32 value, hsd::i32 numerator, hsd::i32 denominator)
{
    return value * numerator / denominator;
}

hsd::i32 shift_value(hsd::i32 value, hsd::i32 offset, hsd::i32 extra)
{
    return value + offset + extra;
}

constexpr hsd::i32 constexpr_add(hsd::i32 lhs, hsd::i32 rhs)
{
    return lhs + rhs;
}

// default_call_t is usable in a constant expression.
constexpr hsd::i32 add_with_defaults()
{
    hsd::default_call_t adder(&constexpr_add, 10, 5);
    return adder(hsd::default_v, 1) + adder(2, hsd::default_v);
}
static_assert(add_with_defaults() == 11 + 7);

// Every stage has the same shape, an int in and an int out, so the stages fit
// in one container of hsd::function.
using stage = hsd::function<hsd::i32(hsd::i32)>;

template <typename Call>
stage make_stage(Call call)
{
    return stage{[call](hsd::i32 value)
    {
        return call(value, hsd::default_v, hsd::default_v);
    }};
}

int main()
{
    hsd::vector<stage> pipeline;

    // Each stage keeps its own fallbacks inside the wrapper. A fallback is
    // stored for every parameter, so the first one is an unused placeholder.
    pipeline.push_back(make_stage(hsd::default_call_t(&scale_value, 0, 3, 2)));
    pipeline.push_back(make_stage(hsd::default_call_t(&shift_value, 0, -4, 0)));
    pipeline.push_back(make_stage(hsd::default_call_t(&clamp_value, 0, 0, 50)));

    const hsd::i32 inputs[] = {-10, 4, 20, 60};

    for (hsd::i32 input : inputs)
    {
        hsd::i32 value = input;

        for (hsd::usize index = 0; index < pipeline.size(); index++)
        {
            // Calling a hsd::function returns a result.
            auto next = pipeline[index](value);

            if (!next.is_ok())
            {
                hsd::println("stage {} failed"_fmt, index);
                return 0;
            }

            value = next.unwrap();
        }

        hsd::println("{} -> {}"_fmt, input, value);
    }

    // The same wrapper can still override a default when a caller wants to.
    hsd::default_call_t clamp(&clamp_value, 0, 0, 100);
    hsd::println("clamp 250 to default max: {}"_fmt, clamp(250, hsd::default_v, hsd::default_v));
    hsd::println("clamp 250 to max 200: {}"_fmt, clamp(250, hsd::default_v, 200));
    hsd::println("constexpr result: {}"_fmt, add_with_defaults());

    return 0;
}
