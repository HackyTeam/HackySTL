#pragma once

#include "Types.hpp"
#include "_Define.hpp"

#include <time.h>
#include <ctime>

namespace hsd
{
    namespace _detail
    {
        struct time_value
        {
            u16 second;
            u16 minute;
            u16 hour;
            u16 month_day;
            u16 week_day;
            u16 month;
            const char* timezone;
            u16 year_day;
            usize year;

            time_value(tm* time_val)
            {
                second = static_cast<u16>(time_val->tm_sec);
                minute = static_cast<u16>(time_val->tm_min);
                hour = static_cast<u16>(time_val->tm_hour);
                month_day = static_cast<u16>(time_val->tm_mday);
                week_day = static_cast<u16>(time_val->tm_wday);
                month = static_cast<u16>(time_val->tm_mon);
                year_day = static_cast<u16>(time_val->tm_yday);
                year = static_cast<usize>(time_val->tm_year);
                timezone = time_val->tm_zone;
            }
        };
        
    } // namespace _detail
    
    class time
    {
    private:
        isize _epoch_time;
        _detail::time_value _time;

    public:
        time()
            : _epoch_time{::time(nullptr)},
            _time{localtime(&_epoch_time)}
        {}

        time& update()
        {
            _epoch_time = ::time(nullptr);
            _time = localtime(&_epoch_time);

            return *this;
        }

        u16 second()
        {
            return _time.second;
        }

        u16 minute()
        {
            return _time.minute;
        }

        u16 hour()
        {
            return _time.hour;
        }

        u16 month_day()
        {
            return _time.month_day;
        }
        
        u16 week_day()
        {
            return _time.week_day;
        }

        u16 month()
        {
            return _time.month;
        }

        const char* timezone()
        {
            return _time.timezone;
        }

        u16 year_day()
        {
            return _time.year_day;
        }

        usize year()
        {
            return _time.year + 1900;
        }
    };

    class clock
    {
    private:
        i64 _clk{};

    public:
        clock(i64 clk = ::clock())
        {
            _clk = clk;
        }

        clock& operator=(const clock& rhs)
        {
            _clk = rhs._clk;
            return *this;
        }

        friend clock operator-(const clock& lhs, const clock& rhs)
        {
            return clock{lhs._clk - rhs._clk};
        }

        friend clock operator+(const clock& lhs, const clock& rhs)
        {
            return clock{lhs._clk + rhs._clk};
        }

        friend clock operator*(const clock& lhs, const clock& rhs)
        {
            return clock{lhs._clk * rhs._clk};
        }

        friend clock operator/(const clock& lhs, const clock& rhs)
        {
            return clock{lhs._clk / rhs._clk};
        }

        bool operator==(const clock& rhs)
        {
            return _clk == rhs._clk;
        }

        bool operator!=(const clock& rhs)
        {
            return _clk != rhs._clk;
        }

        bool operator<(const clock& rhs)
        {
            return _clk < rhs._clk;
        }

        bool operator>(const clock& rhs)
        {
            return _clk > rhs._clk;
        }

        bool operator<=(const clock& rhs)
        {
            return _clk <= rhs._clk;
        }

        bool operator>=(const clock& rhs)
        {
            return _clk >= rhs._clk;
        }

        i64 to_microseconds()
        {
            return _clk;
        }

        i32 to_miliseconds()
        {
            return _clk / 1000;
        }

        f32 to_seconds()
        {
            return static_cast<f32>(_clk) / 1000000;
        }

        clock restart()
        {
            clock _new_clk{::clock() - _clk};
            _clk = ::clock();
            return _new_clk;
        }

        static clock elapsed_time()
        {
            return clock{::clock()};
        }

        static clock zero()
        {
            return clock{0};
        }
    };
} // namespace hsd