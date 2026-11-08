#pragma once

/// @file Time.hpp
/// @brief Calendar dates, time values, clocks, durations, and literals.
/// @details Provides constexpr time-field construction, date refresh, clock
/// arithmetic, timing conversions, and user-defined time literals.

#include "../Containers/String.hpp"

#include <time.h>

#if defined(HSD_PLATFORM_WINDOWS)
#define _WINSOCKAPI_
#include <windows.h>
#endif

namespace hsd
{
    namespace time_detail
    {
        /// @brief Nanoseconds in one second.
        static constexpr int ns_per_second = 1'000'000'000;
        /// @brief Nanoseconds in one millisecond.
        static constexpr int ns_per_ms = 1'000'000;
        /// @brief Nanoseconds in one microsecond.
        static constexpr int ns_per_us = 1'000;
        /// @brief Microseconds in one second.
        static constexpr int us_per_second = 1'000'000;
        /// @brief Microseconds in one millisecond.
        static constexpr int us_per_ms = 1'000;
        /// @brief Milliseconds in one second.
        static constexpr int ms_per_second = 1'000;
        /// @brief Nanoseconds in one 100 ns interval, the unit of Windows file times.
        static constexpr int ns_per_hns = 100;
        /// @brief 100 ns intervals in one second.
        static constexpr i64 hns_per_second = 10'000'000;
        /// @brief 100 ns intervals from 1601-01-01 to 1970-01-01, the Windows file-time epoch.
        static constexpr i64 hns_from_1601_to_1970 = 116'444'736'000'000'000;
        /// @brief Buffer size used for the platform asctime text.
        static constexpr usize asctime_buffer_size = 128;
        /// @brief Fills `spec` with the current wall-clock time.
        /// @details Matches `clock_gettime(CLOCK_REALTIME)` on POSIX. Windows
        /// has no such function in its runtime, so the value comes from the
        /// system file time, which counts 100 ns intervals since 1601.
        /// @param spec Destination receiving seconds and nanoseconds since 1970.
        static inline void realtime_now(timespec& spec)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            i64 _ticks = 0;
            GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&_ticks));

            _ticks -= hns_from_1601_to_1970;
            spec.tv_sec = _ticks / hns_per_second;
            spec.tv_nsec = (_ticks % hns_per_second) * ns_per_hns;
            #else
            clock_gettime(CLOCK_REALTIME, &spec);
            #endif
        }
    } // namespace time_detail

    /// @brief Mutable calendar time fields and time formatting.
    class time
    {
    private:
        /// @brief C library calendar representation retained by this value.
        using time_val = tm;
        /// @brief Calendar month lengths used to validate day updates.
        time_val _time{};
        /// @brief Number of days in each zero-based calendar month.
        static constexpr u8 _month_table[] = {
            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };

        /// @brief Seconds in one minute, which is also the number of `tm_sec` values before the next minute.
        static constexpr int seconds_per_minute = 60;
        /// @brief Minutes in one hour, and the number of `tm_min` values before the next hour.
        static constexpr int minutes_per_hour = 60;
        /// @brief Hours in one day, and the number of `tm_hour` values before the next day.
        static constexpr int hours_per_day = 24;
        /// @brief Months in one year.
        static constexpr int months_per_year = 12;
        /// @brief Zero-based index of February in `_month_table`.
        static constexpr int february_index = 1;
        /// @brief Days in February of a leap year.
        static constexpr int days_in_leap_february = 29;
        /// @brief Offset between the `tm_year` field and the absolute year.
        static constexpr int tm_year_base = 1900;
        /// @brief Radix of the day-of-month part of the chronology key; days run 1 through 31.
        static constexpr i64 day_key_radix = 32;
        /// @brief Radix of the second part of the chronology key; `tm_sec` runs 0 through 60.
        static constexpr i64 second_key_radix = seconds_per_minute + 1;
        /// @brief Calendar month number of March, which starts the year in the day formula.
        static constexpr i64 march_month = 3;
        /// @brief Last month before March; January and February belong to the previous year.
        static constexpr i64 last_month_before_march = 2;
        /// @brief Years in a 400-year Gregorian cycle, which is also the number of years in an era.
        static constexpr i64 years_per_era = 400;
        /// @brief Days in a 400-year Gregorian cycle: 400 * 365 plus 97 leap days.
        static constexpr i64 days_per_400_years = 146097;
        /// @brief Days in a common year.
        static constexpr i64 days_per_common_year = 365;
        /// @brief Years between leap years, outside century years.
        static constexpr i64 leap_year_interval = 4;
        /// @brief Years between century years; a century is a leap year only when divisible by 400.
        static constexpr i64 century_year_interval = 100;
        /// @brief Amount subtracted from a negative year so its division by 400 rounds down.
        static constexpr i64 negative_era_adjust = 399;
        /// @brief Days from 0000-03-01 to 1970-01-01.
        static constexpr i64 unix_epoch_day_offset = 719468;
        /// @brief Slope of the cumulative day count per month in the day formula.
        static constexpr i64 days_before_month_slope = 153;
        /// @brief Rounding bias of the cumulative day count per month in the day formula.
        static constexpr i64 days_before_month_bias = 2;
        /// @brief Divisor of the cumulative day count per month in the day formula.
        static constexpr i64 days_before_month_divisor = 5;
        /// @brief Days in a week.
        static constexpr i64 days_per_week = 7;
        /// @brief Weekday of 1970-01-01, which was a Thursday, with Sunday as 0.
        static constexpr i64 weekday_of_unix_epoch = 4;

    protected:
        /// @brief Replaces calendar fields from a C library time structure.
        /// @param new_val Source calendar value.
        inline void set_time_value(tm* new_val)
        {
            _time = *new_val;
        }

    public:
        /// @brief Constructs a calendar value with day one.
        constexpr time()
        {
            _time.tm_mday = 1;
        }

        /// @brief Returns the seconds field in the range 0 through 59.
        /// @return Seconds field.
        constexpr u16 get_seconds() const
        {
            return static_cast<u16>(_time.tm_sec);
        }

        /// @brief Returns the minutes field in the range 0 through 59.
        /// @return Minutes field.
        constexpr u16 get_minutes() const
        {
            return static_cast<u16>(_time.tm_min);
        }

        /// @brief Returns the hour field in the range 0 through 23.
        /// @return Hour field.
        constexpr u16 get_hour() const
        {
            return static_cast<u16>(_time.tm_hour);
        }

        /// @brief Returns the day-of-month field.
        /// @return Day-of-month field.
        constexpr u16 get_month_day() const
        {
            return static_cast<u16>(_time.tm_mday);
        }

        /// @brief Returns the zero-based month field.
        /// @return Zero-based month field.
        constexpr u16 get_month() const
        {
            return static_cast<u16>(_time.tm_mon);
        }

        /// @brief Returns the calendar year as an absolute year.
        /// @return Absolute calendar year.
        constexpr usize get_year() const
        {
            return static_cast<usize>(_time.tm_year) + tm_year_base;
        }

        /// @brief Days from 1970-01-01 to the given proleptic Gregorian date.
        /// @details Counts whole 400-year eras first, then the years and months inside
        /// the era. Each year starts in March, so January and February count as the last
        /// two months of the previous year.
        /// @param year Full year, such as 2024.
        /// @param month One-based month.
        /// @param day One-based day.
        /// @return Signed day count; negative before the epoch.
        static constexpr i64 _days_from_civil(i64 year, i64 month, i64 day)
        {
            year -= month <= last_month_before_march;
            const i64 _era = (year >= 0 ? year : year - negative_era_adjust) / years_per_era;
            const i64 _year_of_era = year - _era * years_per_era;
            // Months counted from March, so March is 0 and February is 11.
            const i64 _month_index = (month > last_month_before_march)
                ? month - march_month
                : month + months_per_year - march_month;
            const i64 _day_of_year = (days_before_month_slope * _month_index + days_before_month_bias)
                / days_before_month_divisor + day - 1;
            const i64 _day_of_era = _year_of_era * days_per_common_year
                + _year_of_era / leap_year_interval - _year_of_era / century_year_interval
                + _day_of_year;
            return _era * days_per_400_years + _day_of_era - unix_epoch_day_offset;
        }

        /// @brief Recomputes the weekday and day-of-year fields from the date fields.
        /// @details Literals and setters store only the date fields, so this fills
        /// in `tm_wday` and `tm_yday` before they are read.
        constexpr void _update_calendar()
        {
            const i64 _year = static_cast<i64>(_time.tm_year) + tm_year_base;
            const i64 _days = _days_from_civil(_year, _time.tm_mon + 1, _time.tm_mday);
            const i64 _weekday = (_days + weekday_of_unix_epoch) % days_per_week;

            using WeekType = decltype(_time.tm_wday);
            using YearDayType = decltype(_time.tm_yday);
            _time.tm_wday = static_cast<WeekType>(_weekday < 0 ? _weekday + days_per_week : _weekday);
            _time.tm_yday = static_cast<YearDayType>(_days - _days_from_civil(_year, 1, 1));
        }

        /// @brief Orders the date and time fields as one integer.
        /// @return A key that increases with the calendar date and time of day.
        constexpr i64 _chronology_key() const
        {
            i64 _key = _time.tm_year;
            _key = _key * months_per_year + _time.tm_mon;
            _key = _key * day_key_radix + _time.tm_mday;
            _key = _key * hours_per_day + _time.tm_hour;
            _key = _key * minutes_per_hour + _time.tm_min;
            _key = _key * second_key_radix + _time.tm_sec;
            return _key;
        }

        /// @brief Formats the calendar fields using the platform asctime format.
        /// @return Text representation of the calendar value.
        inline string to_text() const
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            char _new_buf[time_detail::asctime_buffer_size]{};
            asctime_s(_new_buf, time_detail::asctime_buffer_size - 1, &_time);
            return {_new_buf};
            #else
            return asctime(&_time);
            #endif
        }

        /// @brief Sets seconds when `seconds < 60`; invalid values are ignored.
        /// @param seconds New seconds field.
        constexpr void set_seconds(u16 seconds)
        {
            if (seconds < seconds_per_minute)
            {
                using CastToType = decltype(_time.tm_sec);
                _time.tm_sec = static_cast<CastToType>(seconds % seconds_per_minute);
            }
        }

        /// @brief Sets minutes when `minutes < 60`; invalid values are ignored.
        /// @param minutes New minutes field.
        constexpr void set_minutes(u16 minutes)
        {
            if (minutes < minutes_per_hour)
            {
                using CastToType = decltype(_time.tm_min);
                _time.tm_min = static_cast<CastToType>(minutes % minutes_per_hour);
            }
        }

        /// @brief Sets hours when `hour < 24`; invalid values are ignored.
        /// @param hour New hour field.
        constexpr void set_hour(u64 hour)
        {
            if (hour < hours_per_day)
            {
                using CastToType = decltype(_time.tm_hour);
                _time.tm_hour = static_cast<CastToType>(hour % hours_per_day);
            }
        }

        /// @brief Sets a day accepted by the current month's table entry.
        /// @param month_day New day-of-month field.
        constexpr void set_month_day(u16 month_day)
        {
            // February may hold day 29; its leap-year check happens when the year is known.
            const u16 _days = (get_month() == february_index) ? days_in_leap_february : _month_table[get_month()];
            if (month_day >= 1 && month_day <= _days)
            {
                using CastToType = decltype(_time.tm_mday);
                _time.tm_mday = static_cast<CastToType>(month_day);
            }
        }

        /// @brief Sets the one-based month input as the C zero-based month field.
        /// @param month One-based month number.
        constexpr void set_month(u16 month)
        {
            if (month >= 1 && month <= months_per_year)
            {
                using CastToType = decltype(_time.tm_mon);
                _time.tm_mon = static_cast<CastToType>(month - 1);
            }
        }

        /// @brief Sets an absolute calendar year.
        /// @param year Absolute year, stored relative to 1900 for `tm`.
        constexpr void set_year(usize year)
        {
            using CastToType = decltype(_time.tm_year);
            _time.tm_year = static_cast<CastToType>(year) - tm_year_base;
        }

        /// @brief Compares all calendar fields for equality.
        /// @param rhs Calendar value compared with this value.
        /// @return True when every calendar field matches.
        constexpr bool operator==(const time& rhs) const
        
        {
            return _chronology_key() == rhs._chronology_key();
        }

        /// @brief Compares all calendar fields for inequality.
        /// @param rhs Calendar value compared with this value.
        /// @return True when any calendar field differs.
        constexpr bool operator!=(const time& rhs) const
        
        {
            return _chronology_key() != rhs._chronology_key();
        }

        /// @brief Orders calendar fields chronologically.
        /// @param rhs Calendar value compared with this value.
        /// @return True when this value precedes `rhs`.
        constexpr bool operator<(const time& rhs) const
        
        {
            return _chronology_key() < rhs._chronology_key();
        }

        /// @brief Tests whether this calendar value is later than `rhs`.
        /// @param rhs Calendar value compared with this value.
        /// @return True when this value follows `rhs`.
        constexpr bool operator>(const time& rhs) const
        
        {
            return _chronology_key() > rhs._chronology_key();
        }

        /// @brief Tests whether this calendar value is no later than `rhs`.
        /// @param rhs Calendar value compared with this value.
        /// @return True when this value does not follow `rhs`.
        constexpr bool operator<=(const time& rhs) const
        
        {
            return _chronology_key() <= rhs._chronology_key();
        }

        /// @brief Tests whether this calendar value is no earlier than `rhs`.
        /// @param rhs Calendar value compared with this value.
        /// @return True when this value does not precede `rhs`.
        constexpr bool operator>=(const time& rhs) const
        
        {
            return _chronology_key() >= rhs._chronology_key();
        }
    };

    /// @brief Current local date value with refresh support.
    class date
        : private time
    {
    private:
        /// @brief Pointer type used with platform localtime functions.
        using time_ptr = tm*;
        /// @brief Epoch timestamp used to refresh the local calendar fields.
        time_t _epoch_date;

    public:
        /// @brief Captures the current local date and time.
        inline date()
            : _epoch_date{::time(nullptr)}
        {
            #ifdef HSD_COMPILER_MSVC
            tm _date_val = {};
            localtime_s(&_date_val, &_epoch_date);
            set_time_value(&_date_val);
            #else
            auto* _date_val = localtime(&_epoch_date);
            set_time_value(_date_val);
            #endif
        }

        /// @brief Refreshes this object from the current local time.
        /// @return `*this` after refreshing the calendar fields.
        inline date& update()
        {
            _epoch_date = ::time(nullptr);
            
            #ifdef HSD_COMPILER_MSVC
            tm _date_val = {};
            localtime_s(&_date_val, &_epoch_date);
            set_time_value(&_date_val);
            #else
            auto* _date_val = localtime(&_epoch_date);
            set_time_value(_date_val);
            #endif
            
            return *this;
        }

        /// @brief Returns the calendar view of this date.
        /// @return Const reference to the inherited time fields.
        inline const time& get_time() const
        {
            return *this;
        }
    };

    /// @brief Process clock value with arithmetic and duration conversion.
    class clock
    {
    private:
        /// @brief Process-clock tick count retained by this value.
        i64 _clk;

    public:
        /// @brief Captures the current process CPU clock.
        inline clock()
            : _clk{::clock()}
        {}

        /// @brief Constructs a clock from native process-clock ticks.
        /// @param clk Native clock tick count.
        explicit constexpr clock(clock_t clk)
        {
            _clk = clk;
        }

        /// @brief Copies a process-clock value.
        /// @param rhs Clock value to copy.
        constexpr clock(const clock& rhs)
            : _clk(rhs._clk)
        {}

        /// @brief Copies a process-clock value.
        /// @param rhs Clock value to copy.
        /// @return `*this` after assignment.
        constexpr clock& operator=(const clock& rhs)
        {
            _clk = rhs._clk;
            return *this;
        }

        /// @brief Returns the elapsed process-clock difference `lhs - rhs`.
        /// @param lhs Left clock value.
        /// @param rhs Right clock value.
        /// @return Difference represented as a clock duration.
        constexpr friend clock operator-(const clock& lhs, const clock& rhs)
        {
            return clock{static_cast<clock_t>(lhs._clk - rhs._clk)};
        }

        /// @brief Adds two native process-clock durations.
        /// @param lhs Left clock value.
        /// @param rhs Right clock value.
        /// @return Sum represented as a clock duration.
        constexpr friend clock operator+(const clock& lhs, const clock& rhs)
        {
            return clock{static_cast<clock_t>(lhs._clk + rhs._clk)};
        }

        /// @brief Tests equality of native process-clock ticks.
        /// @param rhs Clock value compared with this value.
        /// @return True when both native tick counts match.
        constexpr bool operator==(const clock& rhs) const
        {
            return _clk == rhs._clk;
        }

        /// @brief Tests inequality of native process-clock ticks.
        /// @param rhs Clock value compared with this value.
        /// @return True when native tick counts differ.
        constexpr bool operator!=(const clock& rhs) const
        {
            return _clk != rhs._clk;
        }

        /// @brief Tests whether this process-clock value is earlier.
        /// @param rhs Clock value compared with this value.
        /// @return True when this tick count is smaller.
        constexpr bool operator<(const clock& rhs) const
        {
            return _clk < rhs._clk;
        }

        /// @brief Tests whether this process-clock value is later.
        /// @param rhs Clock value compared with this value.
        /// @return True when this tick count is larger.
        constexpr bool operator>(const clock& rhs) const
        {
            return _clk > rhs._clk;
        }

        /// @brief Tests whether this process-clock value is no later.
        /// @param rhs Clock value compared with this value.
        /// @return True when this tick count is no larger.
        constexpr bool operator<=(const clock& rhs) const
        {
            return _clk <= rhs._clk;
        }

        /// @brief Tests whether this process-clock value is no earlier.
        /// @param rhs Clock value compared with this value.
        /// @return True when this tick count is no smaller.
        constexpr bool operator>=(const clock& rhs) const
        {
            return _clk >= rhs._clk;
        }

        /// @brief Converts native ticks to microseconds.
        /// @return Elapsed process CPU time in microseconds.
        constexpr i64 to_microseconds() const
        {
            return static_cast<i64>(_clk) * time_detail::us_per_second / CLOCKS_PER_SEC;
        }

        /// @brief Converts native ticks to milliseconds.
        /// @return Elapsed process CPU time in milliseconds.
        constexpr i32 to_miliseconds() const
        {
            return static_cast<i32>(static_cast<i64>(_clk) * time_detail::ms_per_second / CLOCKS_PER_SEC);
        }

        /// @brief Converts native ticks to seconds.
        /// @return Elapsed process CPU time in seconds.
        constexpr f32 to_seconds() const
        {
            return static_cast<f32>(_clk) / CLOCKS_PER_SEC;
        }

        /// @brief Returns the native process-clock tick count.
        /// @return Stored native tick count.
        constexpr auto to_native() const
        {
            return _clk;
        }

        /// @brief Captures a new current process-clock value.
        /// @return Current process CPU clock.
        static inline auto now()
        {
            return clock{};
        }

        /// @brief Returns elapsed time since this value and restarts the clock.
        /// @return Elapsed duration; this object now contains the current clock.
        inline clock restart()
        {
            clock _old_clk{static_cast<clock_t>(_clk)};
            _clk = ::clock();
            return *this - _old_clk;
        }

        /// @brief Returns elapsed time from this value to the current clock.
        /// @return Current process-clock difference.
        inline clock elapsed_time() const
        {
            clock _new_clk{};
            return _new_clk - *this;
        }
    };

    /// @brief Wall-clock time with nanosecond fields.
    /// @details Reads `CLOCK_REALTIME`, the clock `pthread_cond_timedwait` uses
    /// for absolute deadlines, so `now() + duration` is a valid wait deadline.
    /// The clock follows system time adjustments; use `monotonic_clock` to
    /// measure durations.
    class precise_clock
    {
    private:
        /// @brief Seconds and nanoseconds retained by this clock value.
        timespec _clk;

    public:
        /// @brief Captures the current wall-clock time.
        inline precise_clock()
        {
            time_detail::realtime_now(_clk);
        }

        /// @brief Constructs a precise clock from native timespec fields.
        /// @param clk Seconds/nanoseconds value to copy.
        explicit constexpr precise_clock(const timespec& clk)
            : _clk{clk}
        {}

        /// @brief Copies a precise clock value.
        /// @param rhs Clock value to copy.
        constexpr precise_clock(const precise_clock& rhs)
            : _clk{rhs._clk}
        {}

        /// @brief Copies a precise clock value.
        /// @param rhs Clock value to copy.
        /// @return `*this` after assignment.
        constexpr precise_clock& operator=(const precise_clock& rhs)
        {
            _clk = rhs._clk;
            return *this;
        }

        /// @brief Subtracts timespec values with nanosecond borrowing.
        /// @param lhs Left clock value.
        /// @param rhs Right clock value.
        /// @return Difference represented as a precise duration.
        constexpr friend precise_clock operator-(
            const precise_clock& lhs, const precise_clock& rhs)
        {
            if (lhs._clk.tv_nsec - rhs._clk.tv_nsec < 0)
            {
                timespec _spec{
                    lhs._clk.tv_sec - rhs._clk.tv_sec - 1,
                    lhs._clk.tv_nsec - rhs._clk.tv_nsec + time_detail::ns_per_second
                };

                return precise_clock{_spec};
            }
            else
            {
                timespec _spec{
                    lhs._clk.tv_sec - rhs._clk.tv_sec,
                    lhs._clk.tv_nsec - rhs._clk.tv_nsec  
                };

                return precise_clock{_spec};
            }
        }

        /// @brief Adds timespec values with nanosecond carry handling.
        /// @param lhs Left clock value.
        /// @param rhs Right clock value.
        /// @return Sum represented as a precise duration.
        constexpr friend precise_clock operator+(
            const precise_clock& lhs, const precise_clock& rhs)
        {
            if (lhs._clk.tv_nsec + rhs._clk.tv_nsec >= time_detail::ns_per_second)
            {
                timespec _spec{
                    lhs._clk.tv_sec + rhs._clk.tv_sec + 1,
                    lhs._clk.tv_nsec + rhs._clk.tv_nsec - time_detail::ns_per_second
                };

                return precise_clock{_spec};
            }
            else
            {
                timespec _spec{
                    lhs._clk.tv_sec + rhs._clk.tv_sec,
                    lhs._clk.tv_nsec + rhs._clk.tv_nsec
                };

                return precise_clock{_spec};
            }
        }

        /// @brief Tests equality of seconds and nanoseconds.
        /// @param rhs Clock value compared with this value.
        /// @return True when both timespec fields match.
        constexpr bool operator==(const precise_clock& rhs) const
        {
            return (
                _clk.tv_sec == rhs._clk.tv_sec &&
                _clk.tv_nsec == rhs._clk.tv_nsec
            );
        }

        /// @brief Tests inequality of seconds and nanoseconds.
        /// @param rhs Clock value compared with this value.
        /// @return True when either timespec field differs.
        constexpr bool operator!=(const precise_clock& rhs) const
        {
            return (
                _clk.tv_sec != rhs._clk.tv_sec ||
                _clk.tv_nsec != rhs._clk.tv_nsec
            );
        }

        /// @brief Tests chronological ordering of timespec fields.
        /// @param rhs Clock value compared with this value.
        /// @return True when this value precedes `rhs`.
        constexpr bool operator<(const precise_clock& rhs) const
        {
            if (_clk.tv_sec < rhs._clk.tv_sec)
            {
                return true;
            }
            else if (_clk.tv_sec > rhs._clk.tv_sec)
            {
                return false;
            }
            else
            {
                return _clk.tv_nsec < rhs._clk.tv_nsec;
            }
        }

        /// @brief Tests whether this precise clock is later than `rhs`.
        /// @param rhs Clock value compared with this value.
        /// @return True when this value follows `rhs`.
        constexpr bool operator>(const precise_clock& rhs) const
        {
            if (_clk.tv_sec > rhs._clk.tv_sec)
            {
                return true;
            }
            else if (_clk.tv_sec < rhs._clk.tv_sec)
            {
                return false;
            }
            else
            {
                return _clk.tv_nsec > rhs._clk.tv_nsec;
            }
        }

        /// @brief Tests whether this precise clock is no later than `rhs`.
        /// @param rhs Clock value compared with this value.
        /// @return True when this value does not follow `rhs`.
        constexpr bool operator<=(const precise_clock& rhs) const
        {
            return *this < rhs || *this == rhs;
        }

        /// @brief Tests whether this precise clock is no earlier than `rhs`.
        /// @param rhs Clock value compared with this value.
        /// @return True when this value does not precede `rhs`.
        constexpr bool operator>=(const precise_clock& rhs) const
        {
            return *this > rhs || *this == rhs;
        }

        /// @brief Converts the timespec value to nanoseconds.
        /// @return Elapsed duration in nanoseconds.
        constexpr u64 to_nanoseconds() const
        {
            if (_clk.tv_sec != 0)
            {
                return static_cast<u64>(_clk.tv_sec) * static_cast<u64>(time_detail::ns_per_second)
                    + static_cast<u64>(_clk.tv_nsec);
            }
            else
            {
                return static_cast<u64>(_clk.tv_nsec);
            }
        }

        /// @brief Converts the timespec value to microseconds.
        /// @return Elapsed duration in microseconds.
        constexpr i64 to_microseconds() const
        {
            return static_cast<i64>(to_nanoseconds() / time_detail::ns_per_us);
        }

        /// @brief Converts the timespec value to milliseconds.
        /// @return Elapsed duration in milliseconds.
        constexpr i32 to_miliseconds() const
        {
            return static_cast<i32>(to_nanoseconds() / time_detail::ns_per_ms);
        }

        /// @brief Converts the timespec value to seconds.
        /// @return Elapsed duration in seconds.
        constexpr f64 to_seconds() const
        {
            return static_cast<f64>(_clk.tv_nsec) / 
                static_cast<f64>(time_detail::ns_per_second) + static_cast<f64>(_clk.tv_sec);
        }

        /// @brief Returns the native timespec value.
        /// @return Stored seconds/nanoseconds pair.
        constexpr auto to_native() const
        {
            return _clk;
        }

        /// @brief Captures a new current precise clock value.
        /// @return Current wall-clock time.
        static inline auto now()
        {
            return precise_clock{};
        }

        /// @brief Returns elapsed time since this value and restarts the clock.
        /// @return Elapsed precise duration; this object now contains the current time.
        inline precise_clock restart()
        {
            precise_clock _old_clk{_clk};
            time_detail::realtime_now(_clk);
            return *this - _old_clk;
        }

        /// @brief Returns elapsed time from this value to the current clock.
        /// @return Current precise-clock difference.
        inline precise_clock elapsed_time() const
        {
            precise_clock _new_clk{};
            return _new_clk - *this;
        }
    };

    namespace clock_literals
    {
        /// @brief Creates a process-clock duration from milliseconds.
        /// @param miliseconds Millisecond count represented as native ticks.
        /// @return Process-clock duration.
        static consteval clock operator""_ms(u64 miliseconds)
        {
            return clock{static_cast<clock_t>(miliseconds)};
        }

        /// @brief Creates a process-clock duration from seconds.
        /// @param seconds Second count represented as native ticks.
        /// @return Process-clock duration.
        static consteval clock operator""_s(u64 seconds)
        {
            return clock{static_cast<clock_t>(seconds)};
        }
    } // namespace clock_literals

    namespace precise_clock_literals
    {
        /// @brief Creates a precise duration from nanoseconds.
        /// @param nanoseconds Nanosecond count.
        /// @return Normalized seconds/nanoseconds duration.
        static consteval precise_clock operator""_ns(u64 nanoseconds)
        {
            if (nanoseconds >= time_detail::ns_per_second)
            {
                timespec _spec{
                    .tv_sec = static_cast<time_t>(nanoseconds / time_detail::ns_per_second),
                    .tv_nsec = static_cast<long>(nanoseconds % time_detail::ns_per_second)
                };

                return precise_clock{_spec};
            }
            else
            {
                timespec _spec{
                    .tv_sec = 0,
                    .tv_nsec = static_cast<long>(nanoseconds)
                };

                return precise_clock{_spec};
            }
        }

        /// @brief Creates a precise duration from microseconds.
        /// @param microseconds Microsecond count.
        /// @return Normalized seconds/nanoseconds duration.
        static consteval precise_clock operator""_us(u64 microseconds)
        {
            if (microseconds >= time_detail::us_per_ms)
            {
                if (microseconds >= time_detail::us_per_second)
                {
                    timespec _spec{
                        .tv_sec = static_cast<time_t>(microseconds / time_detail::us_per_second),
                        .tv_nsec = static_cast<long>(microseconds % time_detail::us_per_second) * time_detail::ns_per_us
                    };

                    return precise_clock{_spec};
                }
                else
                {
                    timespec _spec{
                        .tv_sec = 0,
                        .tv_nsec = static_cast<long>(microseconds % time_detail::us_per_second) * time_detail::ns_per_us
                    };

                    return precise_clock{_spec};
                }
            }
            else
            {
                timespec _spec{
                    .tv_sec = 0,
                    .tv_nsec = static_cast<long>(microseconds) * time_detail::ns_per_us
                };

                return precise_clock{_spec};
            }
        }

        /// @brief Creates a precise duration from milliseconds.
        /// @param miliseconds Millisecond count.
        /// @return Normalized seconds/nanoseconds duration.
        static consteval precise_clock operator""_ms(u64 miliseconds)
        {
            if (miliseconds >= time_detail::ms_per_second)
            {
                timespec _spec{
                    .tv_sec = static_cast<time_t>(miliseconds / time_detail::ms_per_second),
                    .tv_nsec = static_cast<long>(miliseconds % time_detail::ms_per_second) * time_detail::ns_per_ms
                };

                return precise_clock{_spec};
            }
            else
            {
                timespec _spec{
                    .tv_sec = 0,
                    .tv_nsec = static_cast<long>(miliseconds) * time_detail::ns_per_ms
                };

                return precise_clock{_spec};
            }
        }

        /// @brief Creates a precise duration from seconds.
        /// @param seconds Second count.
        /// @return Normalized seconds/nanoseconds duration.
        static consteval precise_clock operator""_s(u64 seconds)
        {
            timespec _spec{
                .tv_sec = static_cast<time_t>(seconds),
                .tv_nsec = 0
            };

            return precise_clock{_spec};
        }
    } // namespace precise_clock_literals

    namespace time_literals
    {
        /// @brief Creates a calendar value with a seconds field.
        /// @param seconds Seconds field.
        /// @return Calendar value with the requested seconds.
        static consteval time operator""_s(u64 seconds)
        {
            time _new_time{};
            _new_time.set_seconds(static_cast<u16>(seconds));
            return _new_time;
        }

        /// @brief Creates a calendar value with a minutes field.
        /// @param minutes Minutes field.
        /// @return Calendar value with the requested minutes.
        static consteval time operator""_min(u64 minutes)
        {
            time _new_time{};
            _new_time.set_minutes(static_cast<u16>(minutes));
            return _new_time;
        }

        /// @brief Creates a calendar value with an hour field.
        /// @param hour Hour field.
        /// @return Calendar value with the requested hour.
        static consteval time operator""_h(u64 hour)
        {
            time _new_time{};
            _new_time.set_hour(hour);
            return _new_time;
        }

        /// @brief Creates a calendar value with a day-of-month field.
        /// @param month_day Day-of-month field.
        /// @return Calendar value with the requested day.
        static consteval time operator""_md(u64 month_day)
        {
            time _new_time{};
            _new_time.set_month_day(static_cast<u16>(month_day));
            return _new_time;
        }

        /// @brief Creates a calendar value with a one-based month field.
        /// @param month One-based month field.
        /// @return Calendar value with the requested month.
        static consteval time operator""_m(u64 month)
        {
            time _new_time{};
            _new_time.set_month(static_cast<u16>(month));
            return _new_time;
        }

        /// @brief Creates a calendar value with an absolute year.
        /// @param year Absolute year.
        /// @return Calendar value with the requested year.
        static consteval time operator""_yr(u64 year)
        {
            time _new_time{};
            _new_time.set_year(static_cast<u16>(year));
            return _new_time;
        }

        /// @brief Parses a day-month-year calendar literal with optional HMS fields.
        /// @param date_format Literal date text using dots or slashes.
        /// @return Parsed calendar value.
        static consteval time operator""_dmy(const char* date_format, usize)
        {
            time _new_time{};
            const char* _date_start = date_format;
            char _separator_letter = '.';
            usize _month_day = 0;

            // Day
            {
                // Dot format
                const auto* _day_border = cstring::find(
                    _date_start, _separator_letter
                );

                // Slash format
                if (_day_border == nullptr)
                {
                    // Make sure we are consistent
                    _separator_letter = '/';
                    _day_border = cstring::find(
                        _date_start, _separator_letter
                    );
                }

                // Because we do a month table check after
                _month_day = cstring::parse<usize>(_date_start);
                _date_start = _day_border + 1;
            }

            // Month
            {
                // Dot format
                const auto* _month_border = cstring::find(
                    _date_start, _separator_letter
                );

                _new_time.set_month(
                    cstring::parse<u16>(_date_start)
                );
                
                _new_time.set_month_day(static_cast<u16>(_month_day));
                _date_start = _month_border + 1;
            }

            // Year
            {
                /// Gap for HMS @see line 952
                const auto* _year_border = 
                    cstring::find(_date_start, ' ');

                _new_time.set_year(
                    cstring::parse<usize>(_date_start)
                );
                
                if (_year_border != nullptr)
                    _date_start = _year_border + 1;
            }

            // If we have hours, nimutes and seconds
            if (*(_date_start - 1) == ' ')
            {
                // Hour
                {
                    // Colum format
                    const auto* _hour_border = 
                        cstring::find(_date_start, ':');

                    _new_time.set_hour(
                        cstring::parse<usize>(_date_start)
                    );

                    _date_start = _hour_border + 1;
                }

                // Minutes
                {
                    // Colum format
                    const auto* _min_border = 
                        cstring::find(_date_start, ':');

                    _new_time.set_minutes(
                        cstring::parse<u16>(_date_start)
                    );

                    _date_start = _min_border + 1;
                }

                // Seconds
                {
                    _new_time.set_seconds(
                        cstring::parse<u16>(_date_start)
                    );
                }
            }

            _new_time._update_calendar();
            return _new_time;
        }

        /// @brief Parses a month-day-year calendar literal with optional HMS fields.
        /// @param date_format Literal date text using dots or slashes.
        /// @return Parsed calendar value.
        static consteval time operator""_mdy(const char* date_format, usize)
        {
            time _new_time{};
            const char* _date_start = date_format;
            char _separator_letter = '.';

            // Month
            {
                // Dot format
                const auto* _month_border = cstring::find(
                    _date_start, _separator_letter
                );

                // Slash format
                if (_month_border == nullptr)
                {
                    // Make sure we are consistent
                    _separator_letter = '/';
                    _month_border = cstring::find(
                        _date_start, _separator_letter
                    );
                }

                _new_time.set_month(
                    cstring::parse<u16>(_date_start)
                );

                _date_start = _month_border + 1;
            }

            // Day
            {
                // Dot format
                const auto* _day_border = cstring::find(
                    _date_start, _separator_letter
                );

                _new_time.set_month_day(
                    cstring::parse<u16>(_date_start)
                );
                
                _date_start = _day_border + 1;
            }

            // Year
            {
                /// Gap for HMS @see line 952
                const auto* _year_border = 
                    cstring::find(_date_start, ' ');

                _new_time.set_year(
                    cstring::parse<usize>(_date_start)
                );
                
                if (_year_border != nullptr)
                    _date_start = _year_border + 1;
            }

            // If we have hours, nimutes and seconds
            if (*(_date_start - 1) == ' ')
            {
                // Hour
                {
                    // Colum format
                    const auto* _hour_border = 
                        cstring::find(_date_start, ':');

                    _new_time.set_hour(
                        cstring::parse<usize>(_date_start)
                    );

                    _date_start = _hour_border + 1;
                }

                // Minutes
                {
                    // Colum format
                    const auto* _min_border = 
                        cstring::find(_date_start, ':');

                    _new_time.set_minutes(
                        cstring::parse<u16>(_date_start)
                    );

                    _date_start = _min_border + 1;
                }

                // Seconds
                {
                    _new_time.set_seconds(
                        cstring::parse<u16>(_date_start)
                    );
                }
            }

            _new_time._update_calendar();
            return _new_time;
        }
    } // namespace time_literals
} // namespace hsd