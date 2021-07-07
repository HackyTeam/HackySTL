#include <Time.hpp>
#include <Thread.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::time_literals;
    hsd::date t1;
    hsd::io::print<"Today we are in {}">(t1.get_time().to_text());
    hsd::this_thread::sleep_for(10.f).unwrap();    
    t1.update();
    hsd::io::print<"Today we are in {}">(t1.get_time().to_text());

    // EU date format
    constexpr auto random_date_eu = "28.03.2021 13:43:53"_dmy;
    hsd::io::print<"{}">(random_date_eu.to_text());

    // US date format
    constexpr auto random_date_us = "4.20.2021 12:33:00"_mdy;
    hsd::io::print<"{}">(random_date_us.to_text());
    hsd::io::print<"{}\n">(random_date_eu > random_date_us);
}