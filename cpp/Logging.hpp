#pragma once

#include "Io.hpp"
#include "Time.hpp"

namespace hsd
{
    #if defined(HSD_COMPILER_MSVC)
        #define HSD_FUNCION_NAME __FUNCSIG__
    #elif defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
        #define HSD_FUNCION_NAME __PRETTY_FUNCTION__
    #else
        #define HSD_FUNCION_NAME __builtin_FUNCTION()
    #endif

    namespace _detail
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

        class profiler_value
        {
        private:
            const char* _file_name = "unknown";
            const char* _func = "unknown";
            usize _line = 0;
            clock _clk{};

        public:
            profiler_value(
                const char* file_name = __builtin_FILE(),
                const char* func = __builtin_FUNCTION(), 
                usize line = __builtin_LINE()) noexcept
                : _file_name{file_name}, _func{func}, _line{line}
            {}

            const char* file_name() noexcept
            {
                return _file_name;
            }

            const char* function_name() noexcept
            {
                return _func;
            }

            usize line() noexcept
            {
                return _line;
            }

            clock elapsed_time()
            {
                return _clk.elapsed_time();
            }
        };        
    } // namespace _detail
    
    class stack_trace
    {
    private:
        static inline i32 _count = std::uncaught_exceptions();
        static inline hsd::vector<_detail::source_location> _stack;
        using stack_iterator = hsd::vector<_detail::source_location>::iterator;

    public:

        stack_trace() noexcept = default;
        stack_trace(const stack_trace&) noexcept = default;

        ~stack_trace() noexcept
        {
            if(_count != std::uncaught_exceptions())
            {
                hsd::io::err_print<"Info: {}:{} Function: {}\n">
                ( 
                    get().file_name(), get().line(), 
                    get().function_name()
                );
            }

            _stack.pop_back();
        }

        stack_trace& add(
            const char* func, usize line = __builtin_LINE(),
            const char* file_name = __builtin_FILE()) noexcept
        {
            _stack.emplace_back(file_name, func, line);
            return *this;
        }
        
        _detail::source_location& get() noexcept
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

    class profiler
    {
    private:
        using iterator = hsd::vector<_detail::profiler_value>::iterator;
        static inline hsd::vector<_detail::profiler_value> _stack;

    public:

        profiler() noexcept = default;
        profiler(const profiler&) noexcept = default;

        ~profiler() noexcept
        {
            if(_stack.size() > 0)
            {
                hsd::io::print<"Info: {}:{}\nFunction: {}, time taken: {}us\n">(
                    get().file_name(), get().line(), get().function_name(),
                    get().elapsed_time().to_microseconds()
                );
            }
            _stack.pop_back();
        }

        profiler& add(
            const char* func, usize line = __builtin_LINE(),
            const char* file_name = __builtin_FILE()) noexcept
        {
            _stack.emplace_back(file_name, func, line);
            return *this;
        }
        
        _detail::profiler_value& get() noexcept
        {
            return _stack.back();
        }

        iterator rbegin() noexcept
        {
            return _stack.end() - 1;
        }

        iterator rend() noexcept
        {
            return _stack.begin() - 1;
        }
    };

    static inline stack_trace exec_stack;
    static inline profiler profiler_stack;
    using source_location = _detail::source_location;

    #define invoke_profiler_func(func, ...) func(hsd::profiler_stack.add(HSD_FUNCION_NAME), __VA_ARGS__)
    #define invoke_stacktrace_func(func, ...) func(hsd::exec_stack.add(HSD_FUNCION_NAME), __VA_ARGS__)

    class stack_trace_exception : public std::exception
    {
        virtual const char* what() const noexcept override
        {
            for(auto it = exec_stack.rbegin(); it != exec_stack.rend(); it--)
            {
                hsd::io::err_print<"Info: {}:{} Function: {}\n">
                (
                    it->file_name(), it->line(), 
                    it->function_name()
                );
            }
            
            return "The stack was unwined up there";
        }
    };
} // namespace hsd
