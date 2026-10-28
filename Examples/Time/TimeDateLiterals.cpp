// Title: Date literals
// Level: Basic
// Shows: _dmy and _mdy literals, the time getters, and equality, and compile-time checks.

#include <Serialize/Io.hpp>
#include <Threading/Time.hpp>

using namespace hsd::format_literals;
using namespace hsd::time_literals;

int main()
{
    // Day first: day.month.year hour:minute:second.
    constexpr auto european = "28.03.2022 13:43:53"_dmy;
    hsd::println("european: {}-{}-{} {}:{}:{}"_fmt,
        european.get_year(), european.get_month(), european.get_month_day(),
        european.get_hour(), european.get_minutes(), european.get_seconds());

    // Month first: month.day.year hour:minute:second.
    constexpr auto american = "4.20.2022 12:33:00"_mdy;
    hsd::println("american: {}-{}-{} {}:{}:{}"_fmt,
        american.get_year(), american.get_month(), american.get_month_day(),
        american.get_hour(), american.get_minutes(), american.get_seconds());

    // get_month() counts from zero, so March is 2 and April is 3.
    hsd::println("european month number: {}"_fmt, european.get_month() + 1);

    // The same moment written both ways gives equal values.
    constexpr auto same_day = "03.28.2022 13:43:53"_mdy;
    hsd::println("same moment: {}"_fmt, same_day == european ? "true" : "false");
    hsd::println("different dates: {}"_fmt, european != american ? "true" : "false");

    // The literals are consteval, so the getters can be checked while compiling.
    static_assert(european.get_year() == 2022);
    static_assert(european.get_month() + 1 == 3);
    static_assert(american.get_month() + 1 == 4);
    hsd::println("european month is earlier: {}"_fmt,
        european.get_month() < american.get_month() ? "true" : "false");

    // Single-field literals build a time with only that part set.
    constexpr auto afternoon = 15_h;
    hsd::println("hour literal: {}"_fmt, afternoon.get_hour());

    // Dates order by month, then day, then time, even across a year boundary.
    static_assert(european < american);
    constexpr auto year_end = "31.12.2023 23:59:59"_dmy;
    constexpr auto new_year = "1.1.2024"_dmy;
    hsd::println("december month number: {}"_fmt, year_end.get_month() + 1);
    hsd::println("year end is before new year: {}"_fmt, year_end < new_year ? "true" : "false");

    // The last day of a month is accepted by the literal.
    constexpr auto last_january = "31.1.2024"_dmy;
    hsd::println("last day of january: {}"_fmt, last_january.get_month_day());

    // to_text() prints the weekday, which is set from the date.
    const auto new_year_text = new_year.to_text().sub_string(0, 3).unwrap();
    hsd::println("weekday of 1 January 2024: {}"_fmt, new_year_text);

    return 0;
}
