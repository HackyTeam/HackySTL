#pragma once

#include "Time.hpp"
#include "Vector.hpp"

namespace hsd
{
    namespace detail
    {
        class source_location
        {
        private:
            const char* _file_name;
            const char* _func;
            i32 _line, _column;

        public:
            HSD_CONSTEXPR source_location(
                const char* file_name = __builtin_FILE(),
                const char* func = __builtin_FUNCTION(), 
                i32 line = __builtin_LINE(), i32 column = 0)
                : _file_name{file_name}, _func{func}, 
                    _line{line}, _column{column}
            {}

            constexpr const char* file_name() const
            {
                return _file_name;
            }

            constexpr const char* function_name() const
            {
                return _func;
            }

            constexpr i32 line() const
            {
                return _line;
            }

            constexpr i32 column() const
            {
                return _column;
            }
        };

        class profiler_value
        {
        private:
            source_location _loc;
            precise_clock _clk{};

        public:
            profiler_value(
                const char* file_name = __builtin_FILE(),
                const char* func = __builtin_FUNCTION(), 
                i32 line = __builtin_LINE(), i32 column = 0)
                : _loc{file_name, func, line, column}
            {}

            const char* file_name() const
            {
                return _loc.file_name();
            }

            const char* function_name() const
            {
                return _loc.function_name();
            }

            i32 line() const
            {
                return _loc.line();
            }

            i32 column() const
            {
                return _loc.column();
            }

            precise_clock elapsed_time() const
            {
                return _clk.elapsed_time();
            }
        };        
    } // namespace detail
    
    class stack_trace
    {
    private:
        static inline hsd::vector<detail::source_location> _stack;
        using stack_iterator = hsd::vector<detail::source_location>::iterator;

    public:

        stack_trace() = default;
        stack_trace(const stack_trace&) = default;

        ~stack_trace()
        {
            _stack.pop_back();
        }

        stack_trace& add(
            const char* func, 
            const char* file_name = __builtin_FILE(),
            i32 line = __builtin_LINE(), i32 column = 0)
        {
            _stack.emplace_back(file_name, func, line, column);
            return *this;
        }

        void print_stack()
        {
            for(auto it = _stack.rbegin(); it != _stack.rend(); it--)
            {
                fprintf(
                    stderr, "Info: %s:%d\n\tFunction: %s\n",
                    it->file_name(), it->line(), 
                    it->function_name()
                );
            }
        }
        
        detail::source_location& get() const
        {
            return _stack.back();
        }
    };

    class profiler
    {
    private:
        using iterator = hsd::vector<detail::profiler_value>::iterator;
        static inline hsd::vector<detail::profiler_value> _stack;

    public:

        profiler() = default;
        profiler(const profiler&) = default;

        ~profiler()
        {
            if(_stack.size() > 0)
            {
                printf(
                    "Info: %s:%d\n\tFunction: %s, time taken: %lluus\n", 
                    get().file_name(), get().line(), get().function_name(),
                    get().elapsed_time().to_microseconds()
                );
            }
            _stack.pop_back();
        }

        profiler& add(
            const char* func, usize line = __builtin_LINE(),
            const char* file_name = __builtin_FILE())
        {
            _stack.emplace_back(file_name, func, line);
            return *this;
        }
        
        detail::profiler_value& get() const
        {
            return _stack.back();
        }
    };

    thread_local inline stack_trace exec_stack;
    thread_local inline profiler profiler_stack;
    using source_location = detail::source_location;

    #define invoke_profiler_func(func, ...) \
        func(hsd::profiler_stack.add(HSD_FUNCTION_NAME) __VA_OPT__(,) __VA_ARGS__)
    #define invoke_stacktrace_func(func, ...) \
        func(hsd::exec_stack.add(HSD_FUNCTION_NAME) __VA_OPT__(,) __VA_ARGS__)

    struct stack_trace_error
    {
        const char* operator()() const
        {   
            exec_stack.print_stack();
            fputc('\n', stderr);
            return "The stack was unwined up there";
        }
    };
} // namespace hsd