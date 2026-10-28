#pragma once

/// @file Tracing.hpp
/// @brief Scope timing with serialized, buffered output.
/// @details `trace_scope` times the block it is declared in and emits one
/// `trace_event` when that block ends. Events go to a hook, which by default
/// appends a line to the standard output buffer. Nothing is written until
/// `trace_sink::flush` is called, so tracing does not issue one write per event.

#include "Io.hpp"
#include "../Base/SourceLocation.hpp"
#include "../Threading/Lock.hpp"
#include "../Threading/MonotonicClock.hpp"

namespace hsd
{
    /// @brief One finished scope.
    struct trace_event
    {
        /// @brief Name of the function that contains the scope.
        const char* function;
        /// @brief Source file that declares the scope.
        const char* file;
        /// @brief Source line that declares the scope.
        i32 line;
        /// @brief Nesting level on the emitting thread; 0 is outermost.
        u32 depth;
        /// @brief Monotonic time when the scope started, in nanoseconds.
        u64 start_ns;
        /// @brief Time the scope took, in nanoseconds.
        u64 duration_ns;
    };

    /// @brief Receives finished events.
    /// @details A hook must not open a `trace_scope` of its own, because event
    /// delivery holds a lock that the nested event would need again.
    using trace_hook = void (*)(const trace_event&);

    /// @brief Default hook: appends one line per event to the standard output buffer.
    /// @param ev Event to format.
    inline void print_text(const trace_event& ev)
    {
        using namespace format_literals;

        io::cout().print(
            "{} {}:{} depth {} start {}ns took {}ns\n"_fmt,
            ev.function, ev.file, ev.line, ev.depth, ev.start_ns, ev.duration_ns
        );
    }

    /// @brief Serializes event delivery and owns the active hook.
    class trace_sink
    {
    public:
        /// @brief Replaces the hook that receives events.
        /// @param hook Function called once per event.
        static inline void set_hook(trace_hook hook)
        {
            unique_lock<mutex> _guard{_lock()};
            _active_hook() = hook;
        }

        /// @brief Delivers one event to the active hook.
        /// @details Calls from different threads are serialized, so hooks need
        /// no locking of their own.
        /// @param ev Event to deliver.
        static inline void emit(const trace_event& ev)
        {
            unique_lock<mutex> _guard{_lock()};
            _active_hook()(ev);
        }

        /// @brief Writes buffered output to standard output.
        /// @details Call this before the program exits. Events still buffered
        /// at exit are not written.
        static inline void flush()
        {
            unique_lock<mutex> _guard{_lock()};
            io::cout().flush().unwrap();
        }

    private:
        /// @brief Mutex that serializes hook changes, event delivery, and flushing.
        static inline mutex& _lock()
        {
            static mutex _mtx;
            return _mtx;
        }

        /// @brief Storage for the active hook, initialized to `print_text`.
        static inline trace_hook& _active_hook()
        {
            static trace_hook _hook = &print_text;
            return _hook;
        }
    };

    /// @brief Times the block it is declared in and emits one event when it ends.
    /// @details Nesting is tracked per thread, so concurrent threads never see
    /// each other's depth. A scope cannot be copied or moved, so each scope
    /// emits exactly one event.
    class trace_scope
    {
    public:
        /// @brief Starts timing at the caller's location.
        /// @param loc Declaration site recorded in the event.
        inline trace_scope(source_location loc = source_location::current())
            : _loc{loc}, _depth{_live++}, _start{monotonic_clock::now_ns()}
        {}

        /// @brief Emits the event with its elapsed time and leaves this depth.
        inline ~trace_scope()
        {
            const u64 _end = monotonic_clock::now_ns();
            --_live;

            trace_sink::emit({
                _loc.function_name(), _loc.file_name(), _loc.line(),
                _depth, _start, _end - _start
            });
        }

        /// @brief Scopes emit one event each, so they cannot be copied.
        trace_scope(const trace_scope&) = delete;
        /// @brief Scopes emit one event each, so they cannot be assigned.
        trace_scope& operator=(const trace_scope&) = delete;

    private:
        /// @brief Number of live scopes on the calling thread.
        static inline thread_local u32 _live = 0;

        /// @brief Declaration site of this scope.
        source_location _loc;
        /// @brief Nesting level of this scope on its thread.
        u32 _depth;
        /// @brief Monotonic time when this scope started.
        u64 _start;
    };
} // namespace hsd

#define HSD_TRACE_JOIN_(a, b) a##b
#define HSD_TRACE_JOIN(a, b) HSD_TRACE_JOIN_(a, b)

/// @brief Declares a `trace_scope` for the enclosing block when
/// `HSD_ENABLE_TRACING` is defined, and compiles to nothing otherwise.
#if defined(HSD_ENABLE_TRACING)
#define HSD_TRACE_SCOPE() hsd::trace_scope HSD_TRACE_JOIN(_hsd_trace_, __COUNTER__){}
#else
#define HSD_TRACE_SCOPE() static_cast<void>(0)
#endif
