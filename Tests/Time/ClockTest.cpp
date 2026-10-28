#include <Threading/Time.hpp>
#include <cassert>
#include <chrono>
#include <format>

int main()
{
    using namespace hsd::precise_clock_literals;

    constexpr auto quarter_second = 250'000'000_ns;
    constexpr std::chrono::nanoseconds standard_quarter_second{250'000'000};
    static_assert(quarter_second.to_native().tv_sec == 0);
    static_assert(quarter_second.to_native().tv_nsec == 250'000'000);
    static_assert(quarter_second.to_native().tv_nsec == standard_quarter_second.count());

    constexpr auto half_second = 200'000'000_ns + 300'000'000_ns;
    constexpr std::chrono::nanoseconds standard_half_second{500'000'000};
    static_assert(half_second.to_native().tv_sec == 0);
    static_assert(half_second.to_native().tv_nsec == 500'000'000);
    static_assert(half_second.to_native().tv_nsec == standard_half_second.count());

    constexpr hsd::precise_clock before{timespec{1, 900'000'000}};
    constexpr hsd::precise_clock after{timespec{2, 100'000'000}};
    constexpr auto elapsed = after - before;
    constexpr std::chrono::nanoseconds standard_elapsed{200'000'000};
    static_assert(before < after);
    static_assert(after > before);
    static_assert(elapsed.to_native().tv_sec == 0);
    static_assert(elapsed.to_native().tv_nsec == 200'000'000);
    static_assert(elapsed.to_native().tv_nsec == standard_elapsed.count());

    constexpr hsd::clock one_second{CLOCKS_PER_SEC};
    constexpr std::chrono::seconds standard_one_second{1};
    static_assert(one_second.to_seconds() == 1.0f);
    static_assert(one_second.to_seconds() == standard_one_second.count());
    static_assert(one_second.to_miliseconds() == 1000);
    static_assert(one_second.to_microseconds() == 1'000'000);

    hsd::precise_clock c1;
    c1.restart();
    std::string str;
    for (hsd::u32 i = 0; i < 1000; i++)
    {
        str = std::format("Hello World {} a {}\r", 123231u, 4.232f);
    }

    assert(str.starts_with("Hello World 123231"));
    assert(c1.restart().to_nanoseconds() > 0);
    assert(hsd::clock{}.to_seconds() >= 0.0f);

    return 0;
}