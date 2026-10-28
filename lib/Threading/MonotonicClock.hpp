#pragma once

/// @file MonotonicClock.hpp
/// @brief Elapsed-time source that never moves backwards.
/// @details Use `monotonic_clock` to measure durations. `precise_clock` is
/// wall-clock time and can jump when the system clock is adjusted.

#include "../Base/Types.hpp"

#if defined(HSD_PLATFORM_WINDOWS)
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <time.h>
#endif

namespace hsd
{
    /// @brief Monotonic nanosecond counter for measuring elapsed time.
    class monotonic_clock
    {
    public:
        /// @brief Returns the current monotonic time in nanoseconds.
        /// @details The origin is unspecified; only differences are meaningful.
        /// @return Nanoseconds since an unspecified starting point.
        static inline u64 now_ns()
        {
#if defined(HSD_PLATFORM_WINDOWS)
            LARGE_INTEGER _freq{};
            LARGE_INTEGER _count{};
            QueryPerformanceFrequency(&_freq);
            QueryPerformanceCounter(&_count);

            // Divide before multiplying: `ticks * 1e9` overflows u64 after
            // about 30 minutes on a 10 MHz counter.
            const u64 _ticks = static_cast<u64>(_count.QuadPart);
            const u64 _hz = static_cast<u64>(_freq.QuadPart);
            return (_ticks / _hz) * 1'000'000'000
                + (_ticks % _hz) * 1'000'000'000 / _hz;
#else
            timespec _ts{};
            clock_gettime(CLOCK_MONOTONIC, &_ts);
            return static_cast<u64>(_ts.tv_sec) * 1'000'000'000
                + static_cast<u64>(_ts.tv_nsec);
#endif
        }
    };
} // namespace hsd
