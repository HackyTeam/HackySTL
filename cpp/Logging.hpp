#pragma once

#include "Io.hpp"

namespace hsd
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
    
    class stack_trace
    {
    private:
        static inline hsd::vector<source_location> _stack;

    public:
        using stack_iterator = hsd::vector<source_location>::iterator;

        static constexpr void add(const char* file_name, const char* func, size_t line)
        {
            _stack.emplace_back(file_name, func, line);
        }

        static constexpr void remove()
        {
            _stack.pop_back();
        }
        
        static constexpr source_location get()
        {
            return _stack.back();
        }

        static constexpr stack_iterator begin()
        {
            return _stack.begin();
        }

        static constexpr stack_iterator end()
        {
            return _stack.end();
        }
    };

    #define HSD_PRINT_STACK()\
        for(hsd::stack_trace::stack_iterator it = stack_trace::begin(); it != stack_trace::end(); it++)\
            hsd::io::print("Info: {}:{} Function: {}\n", it->file_name(), it->line(), it->function_name())

    #define HSD_STACKTRACE_FUNC(func, ...)\
        hsd::stack_trace::add(__FILE__, __func__, __LINE__);\
        func(__VA_ARGS__);\
        hsd::stack_trace::remove()

    class stack_trace_exception : public std::exception
    {
        virtual const char* what() const noexcept override
        {
            HSD_PRINT_STACK();
            return "The stack was unwined up there";
        }
    };
} // namespace hsd
