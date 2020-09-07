#pragma once

#include "Types.hpp"

namespace hsd
{
    class logger
    {
    private:
        const char* _file_name;
        const char* _func;
        size_t _line;

    public:
        constexpr logger()
            : _file_name{"unknown"}, _func{"unknown"}, _line{0}
        {}

        constexpr logger(const logger& log)
            : _file_name{log._file_name}, _func{log._func}, _line{log._line}
        {}

        constexpr logger(logger&& log)
            : _file_name{log._file_name}, _func{log._func}, _line{log._line}
        {
            log._file_name = nullptr;
            log._func = nullptr;
            log._line = 0;
        }

        static constexpr logger current(const char* file_name = __builtin_FILE(), 
            const char* func = __builtin_FUNCTION(), size_t line = __builtin_LINE())
        {
            logger _log;
            _log._file_name = file_name;
            _log._func = func;
            _log._line = line;
            return _log;
        }
        
        constexpr const char* file_name()
        {
            return _file_name;
        }

        constexpr const char* function_name()
        {
            return _func;
        }

        constexpr size_t line()
        {
            return _line;
        }
    };
    
} // namespace hsd
