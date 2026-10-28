// Title: Compile-time tuple apply
// Level: Advanced
// Shows: constexpr tuples, hsd::apply with free callables and member functions, and static_assert checks.

#include <Serialize/Io.hpp>
#include <Wrappers/Tuple.hpp>

using namespace hsd::format_literals;

static constexpr hsd::i32 volume(hsd::i32 width, hsd::i32 height, hsd::i32 depth)
{
    return width * height * depth;
}

struct accumulator
{
    hsd::i32 total = 0;

    constexpr hsd::i32 add(hsd::i32 left, hsd::i32 right)
    {
        total += left + right;
        return total;
    }
};

// Everything below is computed by the compiler. Tuples built from literals are
// constexpr, and apply expands a tuple into the arguments of a call.
static constexpr hsd::tuple box = {2, 3, 4};
static constexpr hsd::tuple unit_box = {1, 1, 1};

static_assert(box.get<0>() == 2 && box.get<2>() == 4);
static_assert(hsd::apply(volume, box) == 24);
static_assert(hsd::apply(volume, unit_box) == 1);

// operator+ concatenates, so the result has six elements.
static constexpr auto both = box + unit_box;
static_assert(both.get<0>() == 2 && both.get<3>() == 1 && both.get<5>() == 1);

// A lambda works as the callable too.
static constexpr auto sum_of_three = [](hsd::i32 a, hsd::i32 b, hsd::i32 c) { return a + b + c; };
static_assert(hsd::apply(sum_of_three, box) == 9);

// The same pipeline in a constexpr function, to show apply inside a constant expression.
static constexpr hsd::i32 total_volume()
{
    hsd::tuple wide = {5, 1, 1};
    return hsd::apply(volume, box) + hsd::apply(volume, wide);
}

static_assert(total_volume() == 29);

// Member functions take the object first, then the argument tuple.
static constexpr hsd::i32 accumulate_pairs()
{
    accumulator acc;
    acc.add(1, 2);
    hsd::tuple args = {10, 20};
    return hsd::apply(&accumulator::add, acc, args);
}

static_assert(accumulate_pairs() == 33);

int main()
{
    hsd::println("volume of box: {}"_fmt, hsd::apply(volume, box));
    hsd::println("sum of box: {}"_fmt, hsd::apply(sum_of_three, box));
    hsd::println("total volume: {}"_fmt, total_volume());
    hsd::println("six elements: {} {} {} {} {} {}"_fmt,
        both.get<0>(), both.get<1>(), both.get<2>(),
        both.get<3>(), both.get<4>(), both.get<5>());

    // At run time apply works the same way, here with a runtime object.
    accumulator acc;
    hsd::tuple first_call = {5, 6};
    hsd::tuple second_call = {7, 8};
    hsd::apply(&accumulator::add, acc, first_call);
    hsd::println("running total: {}"_fmt, hsd::apply(&accumulator::add, acc, second_call));
    hsd::println("accumulate_pairs: {}"_fmt, accumulate_pairs());
    return 0;
}
