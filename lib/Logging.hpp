#pragma once

#include "Time.hpp"
#include "Vector.hpp"

namespace hsd
{
    namespace logger_detail
    {
        class source_location
        {
        private:
            const char* _file_name;
            const char* _func;
            i32 _line, _column;

        public:
            constexpr source_location(
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
            inline profiler_value(
                const char* file_name = __builtin_FILE(),
                const char* func = __builtin_FUNCTION(), 
                i32 line = __builtin_LINE(), i32 column = 0)
                : _loc{file_name, func, line, column}
            {}

            inline const char* file_name() const
            {
                return _loc.file_name();
            }

            inline const char* function_name() const
            {
                return _loc.function_name();
            }

            inline i32 line() const
            {
                return _loc.line();
            }

            inline i32 column() const
            {
                return _loc.column();
            }

            inline precise_clock elapsed_time() const
            {
                return _clk.elapsed_time();
            }
        };        
    } // namespace logger_detail
    
    class stack_trace
    {
    private:
        using stack_type = hsd::vector<logger_detail::source_location>;
        using stack_iterator = typename stack_type::iterator;

        stack_type _stack;

    public:

        inline stack_trace() = default;
        inline stack_trace(const stack_trace&) = default;

        inline ~stack_trace()
        {
            _stack.pop_back();
        }

        inline stack_trace& add(
            const char* func, 
            const char* file_name = __builtin_FILE(),
            i32 line = __builtin_LINE(), i32 column = 0)
        {
            _stack.emplace_back(file_name, func, line, column);
            return *this;
        }

        inline void print_stack()
        {
            for (auto it = _stack.rbegin(); it != _stack.rend(); it--)
            {
                fprintf(
                    stderr, "Info: %s:%d\n\tFunction: %s\n",
                    it->file_name(), it->line(), 
                    it->function_name()
                );
            }
        }
        
        inline const logger_detail::source_location& get() const
        {
            return _stack.back();
        }
    };

    class profiler
    {
    private:
        using profiler_type = hsd::vector<logger_detail::profiler_value>;
        using profiler_iterator = typename profiler_type::iterator;

        profiler_type _stack;

    public:

        inline profiler() = default;
        inline profiler(const profiler&) = default;

        inline ~profiler()
        {
            if(_stack.size() > 0)
            {
                printf(
                    "Info: %s:%d\n\tFunction: %s, time taken: %llu\n", 
                    get().file_name(), get().line(), get().function_name(),
                    get().elapsed_time().to_microseconds()
                );
            }
            _stack.pop_back();
        }

        inline profiler& add(
            const char* func, i32 line = __builtin_LINE(),
            const char* file_name = __builtin_FILE())
        {
            _stack.emplace_back(file_name, func, line);
            return *this;
        }
        
        inline const logger_detail::profiler_value& get() const
        {
            return _stack.back();
        }
    };

    static inline stack_trace exec_stack;
    static inline profiler profiler_stack;
    using source_location = logger_detail::source_location;

    #define invoke_profiler_func(func, ...) \
        func(hsd::profiler_stack.add(HSD_FUNCTION_NAME) __VA_OPT__(,) __VA_ARGS__)
    #define invoke_stacktrace_func(func, ...) \
        func(hsd::exec_stack.add(HSD_FUNCTION_NAME) __VA_OPT__(,) __VA_ARGS__)

    struct stack_trace_error
    {
        inline const char* operator()() const
        {   
            exec_stack.print_stack();
            fputc('\n', stderr);
            return "The stack was unwind up there";
        }
    };
} // namespace hsd