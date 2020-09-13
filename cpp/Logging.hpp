#pragma once

#include "Io.hpp"

namespace hsd
{
    class source_location
    {
    private:
        const char* _file_name = "unknown";
        const char* _func = "unknown";
        usize _line = 0;
    
    public:
        constexpr source_location(
            const char* file_name = __builtin_FILE(), 
            const char* func = __builtin_FUNCTION(), 
            usize line = __builtin_LINE()) noexcept
            : _file_name{file_name}, _func{func}, _line{line}
        {}
        
        constexpr const char* file_name() noexcept
        {
            return _file_name;
        }
        
        constexpr const char* function_name() noexcept
        {
            return _func;
        }
        
        constexpr usize line() noexcept
        {
            return _line;
        }
    };
    
    class stack_trace
    {
    private:
        static inline hsd::vector<source_location> _stack;
        static inline i32 _count = std::uncaught_exceptions();
        using stack_iterator = hsd::vector<source_location>::iterator;

    public:

        stack_trace() noexcept {}
        stack_trace(const stack_trace&) noexcept {}

        ~stack_trace() noexcept
        {
            if(_count != std::uncaught_exceptions())
            {
                hsd::io::err_print(
                    "Info: {}:{} Function: {}\n", 
                    get().file_name(), get().line(), 
                    get().function_name()
                );
            }

            _stack.pop_back();
        }

        stack_trace& add(
            const char* file_name = __builtin_FILE(), 
            const char* func = __builtin_FUNCTION(), 
            usize line = __builtin_LINE()) noexcept
        {
            _stack.emplace_back(file_name, func, line);
            return *this;
        }
        
        constexpr source_location get() noexcept
        {
            return _stack.back();
        }

        stack_iterator rbegin() noexcept
        {
            return _stack.end() - 1;
        }

        stack_iterator rend() noexcept
        {
            return _stack.begin() - 1;
        }
    };

    static inline stack_trace exec_stack;

    class stack_trace_exception : public std::exception
    {
        virtual const char* what() const noexcept override
        {
            for(auto it = exec_stack.rbegin(); it != exec_stack.rend(); it--)
            {
                hsd::io::err_print(
                    "Info: {}:{} Function: {}\n", 
                    it->file_name(), it->line(), 
                    it->function_name()
                );
            }
            
            return "The stack was unwined up there";
        }
    };
} // namespace hsd
