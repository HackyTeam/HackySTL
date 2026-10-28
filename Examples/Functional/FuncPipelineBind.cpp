// Title: Pipelines and binding
// Level: Advanced
// Shows: factories returning hsd::function, partial application with captures, composition, and a stage pipeline.

#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Wrappers/Functional.hpp>

using namespace hsd::format_literals;

using unary = hsd::function<hsd::i32(hsd::i32)>;

// There is no bind helper: partial application is a lambda that captures the fixed argument.
static unary make_adder(hsd::i32 amount)
{
    return [amount](hsd::i32 value) { return value + amount; };
}

static unary make_multiplier(hsd::i32 factor)
{
    return [factor](hsd::i32 value) { return value * factor; };
}

// Composition: run `first`, then feed the answer to `second`.
// Both functions are copied into the new closure, so they outlive the arguments.
static unary compose(unary first, unary second)
{
    return [first, second](hsd::i32 value) mutable
    {
        // An empty stage yields an error; treat it as "leave the value alone".
        auto middle = first(value);
        hsd::i32 passed = middle.is_ok() ? middle.unwrap() : value;
        return second(passed).unwrap_or(passed);
    };
}

// Currying a two-argument function into a chain of one-argument ones.
static hsd::function<unary(hsd::i32)> curry(hsd::i32 (*binary)(hsd::i32, hsd::i32))
{
    return [binary](hsd::i32 left)
    {
        return unary{[binary, left](hsd::i32 right) { return binary(left, right); }};
    };
}

static hsd::i32 subtract(hsd::i32 left, hsd::i32 right)
{
    return left - right;
}

// A pipeline applies its stages in order.
class pipeline
{
private:
    hsd::vector<unary> _stages;

public:
    pipeline& then(unary stage)
    {
        _stages.push_back(hsd::move(stage));
        return *this;
    }

    hsd::i32 run(hsd::i32 value)
    {
        for (hsd::usize index = 0; index < _stages.size(); ++index)
        {
            value = _stages[index](value).unwrap();
        }

        return value;
    }
};

int main()
{
    unary add_five = make_adder(5);
    unary times_three = make_multiplier(3);
    hsd::println("add_five(10) = {}"_fmt, add_five(10).unwrap());
    hsd::println("times_three(10) = {}"_fmt, times_three(10).unwrap());

    // The order of composition matters.
    unary add_then_times = compose(add_five, times_three);
    unary times_then_add = compose(times_three, add_five);
    hsd::println("(10 + 5) * 3 = {}"_fmt, add_then_times(10).unwrap());
    hsd::println("(10 * 3) + 5 = {}"_fmt, times_then_add(10).unwrap());

    // An empty stage is skipped by compose.
    unary skipped = compose(unary{}, add_five);
    hsd::println("empty then add_five on 1 = {}"_fmt, skipped(1).unwrap());

    auto subtract_from = curry(subtract);
    unary ten_minus = subtract_from(10).unwrap();
    hsd::println("10 - 3 = {}"_fmt, ten_minus(3).unwrap());
    hsd::println("10 - 25 = {}"_fmt, ten_minus(25).unwrap());

    // A pipeline of stages, including one with state: it counts how often it ran.
    hsd::i32 calls = 0;
    pipeline steps;
    steps.then(make_adder(1))
        .then(make_multiplier(2))
        .then([&calls](hsd::i32 value) { ++calls; return value - 3; })
        .then(compose(make_adder(100), make_multiplier(-1)));

    for (hsd::i32 input = 0; input < 4; ++input)
    {
        hsd::i32 output = steps.run(input);
        hsd::println("pipeline({}) = {}"_fmt, input, output);
    }

    hsd::println("stateful stage ran {} times"_fmt, calls);
    return 0;
}
