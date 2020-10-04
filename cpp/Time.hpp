#pragma once

#include "Types.hpp"
#include "_Define.hpp"

#include <time.h>
#include <wchar.h>

namespace hsd
{  
    class time
    {
    private:
        using time_ptr = tm*;
        isize _epoch_time;
        time_ptr _time;

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
            return _time->tm_sec;
        }

        u16 minute()
        {
            return _time->tm_min;
        }

        u16 hour()
        {
            return _time->tm_hour;
        }

        u16 month_day()
        {
            return _time->tm_mday;
        }
        
        u16 week_day()
        {
            return _time->tm_wday;
        }

        u16 month()
        {
            return _time->tm_mon;
        }

        const char* timezone()
        {
            return _time->tm_zone;
        }

        u16 year_day()
        {
            return _time->tm_yday;
        }

        usize year()
        {
            return _time->tm_year + 1900;
        }

        const char* to_text()
        {
            return asctime(_time);
        }
    };

    class clock
    {
    private:
        i64 _clk;

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