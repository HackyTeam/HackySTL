#include <cassert>

#include <Threading/Time.hpp>

using namespace hsd::time_literals;

// Regression tests for calendar ordering and the date setters: months compare
// chronologically, December and the last day of a month are accepted, and the
// weekday printed by to_text() matches the date.
static_assert("28.03.2022 13:43:53"_dmy < "4.20.2022 12:33:00"_mdy);
static_assert("4.20.2022 12:33:00"_mdy > "28.03.2022 13:43:53"_dmy);
static_assert("1.1.2024"_dmy > "31.12.2023 23:59:59"_dmy);
static_assert("31.12.2023 23:59:59"_dmy < "1.1.2024"_dmy);
static_assert("5.6.2024"_dmy == "6.5.2024"_mdy);
static_assert("5.6.2024"_dmy != "6.5.2024 00:00:01"_mdy);
static_assert("1.1.2024 10:00:00"_dmy < "1.1.2024 11:00:00"_dmy);
static_assert("1.1.2024 10:59:59"_dmy < "1.1.2024 11:00:00"_dmy);
static_assert("1.2.2024"_dmy < "1.3.2024"_dmy);
static_assert("31.1.2024"_dmy < "1.2.2024"_dmy);

int main()
{
    constexpr auto december = "31.12.2023 23:59:59"_dmy;
    static_assert(december.get_month() == 11);
    assert(december.get_month_day() == 31);
    assert(december.get_year() == 2023);

    constexpr auto last_day = "31.1.2024"_dmy;
    assert(last_day.get_month() == 0);
    assert(last_day.get_month_day() == 31);

    // 1 January 2024 was a Monday.
    const auto monday = "1.1.2024"_dmy;
    const auto text = monday.to_text();
    assert(text.length() >= 3);
    assert(text.data()[0] == 'M' && text.data()[1] == 'o' && text.data()[2] == 'n');

    // The same date written in both orders is the same value.
    const auto day_first = "5.6.2024"_dmy;
    const auto month_first = "6.5.2024"_mdy;
    assert(day_first == month_first);
    assert(!(day_first != month_first));
}
