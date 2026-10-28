#include <Threading/Time.hpp>
#include <Threading/Thread.hpp>
#include <cassert>

int main()
{
    using namespace hsd::format_literals;
    using namespace hsd::time_literals;

    hsd::date t1;
    assert(t1.get_time().get_year() >= 2026);
    assert(hsd::this_thread::sleep_for(0.001f).is_ok());
    t1.update();
    assert(t1.get_time().to_text().length() > 0);

    // EU date format
    constexpr auto random_date_eu = "28.03.2022 13:43:53"_dmy;
    assert(random_date_eu.get_month_day() == 28);
    assert(random_date_eu.get_month() == 2);
    assert(random_date_eu.get_year() == 2022);
    assert(random_date_eu.get_hour() == 13);
    assert(random_date_eu.get_minutes() == 43);
    assert(random_date_eu.get_seconds() == 53);

    // US date format
    constexpr auto random_date_us = "4.20.2022 12:33:00"_mdy;
    assert(random_date_us.get_month_day() == 20);
    assert(random_date_us.get_month() == 3);
    assert(random_date_us.get_year() == 2022);
    assert(random_date_us.get_hour() == 12);
    assert(random_date_us.get_minutes() == 33);
    assert(random_date_us.get_seconds() == 0);
    assert(random_date_eu != random_date_us);
}