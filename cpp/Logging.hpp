#pragma once

#include "Types.hpp"
#include "Vector.hpp"

namespace hsd
{
    namespace logger_detail
    {
        class source_location
        {
        private:
            const char* _file_name = "unknown";
            const char* _func = "unknown";
            size_t _line = 0;
        
        public:
            constexpr source_location() {}

            constexpr source_location(const char* file_name, const char* func, size_t line)
                : _file_name{file_name}, _func{func}, _line{line}
            {}

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
    } // namespace logger_detail
    
    class logger
    {
    private:
        hsd::vector<logger_detail::source_location> _stack;
        logger* _ptr = nullptr;
        using stack_iterator = hsd::vector<logger_detail::source_location>::iterator;

    public:
        constexpr logger() {}

        constexpr logger(logger& log)
            : _ptr{&log}
        {}

        constexpr ~logger()
        {
            if(_ptr != nullptr)
                _ptr->_stack.pop_back();
        }

        constexpr logger& add(const char* file_name = __builtin_FILE(), 
            const char* func = __builtin_FUNCTION(), size_t line = __builtin_LINE())
        {
            if(_ptr != nullptr) {
                _ptr->add(file_name, func, line);
            }
            else {
                _stack.emplace_back(file_name, func, line);
            }

            return *this;
        }
        
        constexpr logger_detail::source_location get()
        {
            if(_ptr != nullptr)
                return _ptr->get();
    
            return _stack.back();
        }

        constexpr stack_iterator begin()
        {
            if(_ptr != nullptr)
                return _ptr->begin();

            return _stack.begin();
        }

        constexpr stack_iterator begin() const
        {
            if(_ptr != nullptr)
                return _ptr->begin();

            return _stack.begin();
        }

        constexpr stack_iterator end()
        {
            if(_ptr != nullptr)
                return _ptr->end();

            return _stack.end();
        }

        constexpr stack_iterator end() const
        {
            if(_ptr != nullptr)
                return _ptr->end();

            return _stack.end();
        }
    };
} // namespace hsd
