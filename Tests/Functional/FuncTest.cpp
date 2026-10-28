#include <Serialize/Io.hpp>
#include <Wrappers/Functional.hpp>
#include <Threading/Future.hpp>
#include <cassert>
#include <functional>
#include <future>

struct large_callable
{
    char storage[64]{};

    hsd::i32 operator()(hsd::i32 value) const
    {
        return value + 7;
    }
};

static void test_function_against_std()
{
    hsd::function<hsd::i32(hsd::i32)> actual = [](hsd::i32 value)
    {
        return value * 3 + 2;
    };
    std::function<hsd::i32(hsd::i32)> expected = [](hsd::i32 value)
    {
        return value * 3 + 2;
    };

    assert(static_cast<bool>(actual) == static_cast<bool>(expected));
    for (hsd::i32 value : {-10, -1, 0, 1, 17})
    {
        assert(actual(value).unwrap() == expected(value));
    }

    auto actual_copy = actual;
    auto expected_copy = expected;
    assert(actual_copy(9).unwrap() == expected_copy(9));

    hsd::function<hsd::i32(hsd::i32)> actual_moved = hsd::move(actual_copy);
    std::function<hsd::i32(hsd::i32)> expected_moved = std::move(expected_copy);
    assert(actual_moved(20).unwrap() == expected_moved(20));

    hsd::function<hsd::i32()> actual_stateful = [count = 0]() mutable
    {
        return ++count;
    };
    std::function<hsd::i32()> expected_stateful = [count = 0]() mutable
    {
        return ++count;
    };
    assert(actual_stateful().unwrap() == expected_stateful());
    auto actual_stateful_copy = actual_stateful;
    auto expected_stateful_copy = expected_stateful;
    assert(actual_stateful().unwrap() == expected_stateful());
    assert(actual_stateful_copy().unwrap() == expected_stateful_copy());

    hsd::function<hsd::i32(hsd::i32)> actual_large{large_callable{}};
    std::function<hsd::i32(hsd::i32)> expected_large{large_callable{}};
    for (hsd::i32 value : {0, 5, 42})
    {
        assert(actual_large(value).unwrap() == expected_large(value));
    }
    auto actual_large_copy = actual_large;
    auto expected_large_copy = expected_large;
    assert(actual_large_copy(10).unwrap() == expected_large_copy(10));

    hsd::function<void()> empty_actual;
    std::function<void()> empty_expected;
    assert(static_cast<bool>(empty_actual) == static_cast<bool>(empty_expected));
    assert(!empty_actual());

    hsd::function<hsd::i32()> empty_value_actual;
    assert(!empty_value_actual);
    assert(!empty_value_actual().is_ok());

    hsd::i32 actual_side_effect = 0;
    hsd::i32 expected_side_effect = 0;
    hsd::function<void(hsd::i32)> actual_void = [&actual_side_effect](hsd::i32 value)
    {
        actual_side_effect += value;
    };
    std::function<void(hsd::i32)> expected_void = [&expected_side_effect](hsd::i32 value)
    {
        expected_side_effect += value;
    };
    actual_void(4).unwrap();
    expected_void(4);
    actual_void(-7).unwrap();
    expected_void(-7);
    assert(actual_side_effect == expected_side_effect);

    hsd::function<hsd::i32()> actual_result = []
    {
        return hsd::result<hsd::i32, hsd::runtime_error>{23};
    };
    std::function<hsd::i32()> expected_result = [] { return hsd::i32{23}; };
    assert(actual_result().unwrap() == expected_result());
}

static void test_packaged_task_against_std()
{
    hsd::packaged_task<hsd::i32(hsd::i32)> actual_task{
        [](hsd::i32 value) { return value * 2 + 1; }
    };
    std::packaged_task<hsd::i32(hsd::i32)> expected_task{
        [](hsd::i32 value) { return value * 2 + 1; }
    };
    auto actual_future = actual_task.get_future();
    auto expected_future = expected_task.get_future();

    actual_task(21);
    expected_task(21);
    assert(actual_future.get().unwrap() == expected_future.get());
}

int main()
{
    using namespace hsd::format_literals;

    test_function_against_std();
    test_packaged_task_against_std();

    hsd::packaged_task<hsd::f64()> packaged_task([](){ return 3.14159; });

    hsd::future<hsd::f64> future = packaged_task.get_future();

    auto lambda = [task = hsd::move(packaged_task)]() mutable { task(); };

    hsd::function<void()> function = hsd::move(lambda); // Ok

    function().unwrap();

    assert(future.get().unwrap() == 3.14159);

}