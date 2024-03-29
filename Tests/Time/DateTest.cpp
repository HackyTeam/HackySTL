#include <Time.hpp>
#include <Thread.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::format_literals;
    using namespace hsd::time_literals;

    hsd::date t1;
    hsd::print("Today we are in {}"_fmt, t1.get_time().to_text());
    hsd::this_thread::sleep_for(10.f).unwrap();    
    t1.update();
    hsd::print("Today we are in {}"_fmt, t1.get_time().to_text());

    // EU date format
    constexpr auto random_date_eu = "28.03.2022 13:43:53"_dmy;
    hsd::print("{}"_fmt, random_date_eu.to_text());

    // US date format
    constexpr auto random_date_us = "4.20.2022 12:33:00"_mdy;
    hsd::print("{}"_fmt, random_date_us.to_text());
    hsd::println("{}"_fmt, (random_date_eu > random_date_us) ? "true" : "false");
}