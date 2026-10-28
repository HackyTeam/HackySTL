#pragma once

/// @file Logging.hpp
/// @brief Call-site stack tracing and diagnostic logging utilities.
/// @details Provides source-aware reporting helpers used by tests and
/// diagnostic code without exception-based propagation. Scope timing lives in
/// Tracing.hpp.

#include "Io.hpp"
#include "../Containers/Range.hpp"

#include "../Base/SourceLocation.hpp"

namespace hsd
{
    /// @brief RAII marker that records the current call site on the calling thread's stack.
    /// @details `add` pushes the caller's location onto a stack local to each
    /// thread and returns a handle that owns that entry. Destroying the handle
    /// pops it. Handles cannot be copied or moved, so every entry is popped
    /// exactly once and in LIFO order, matching normal scope nesting. A
    /// default-constructed handle owns no entry.
    class stack_trace
    {
    private:
        /// @brief Storage type for the call-site stack.
        using stack_type = hsd::vector<source_location>;

        /// @brief Call-site stack of the calling thread.
        static inline thread_local stack_type _stack;

        /// @brief True when this handle owns the entry pushed by `add`.
        bool _owns = false;

        /// @brief Creates a handle with the given ownership.
        /// @param owns Whether the handle owns an entry on the stack.
        explicit inline stack_trace(bool owns)
            : _owns{owns}
        {}

    public:
        /// @brief Constructs a handle that owns no entry.
        inline stack_trace() = default;

        /// @brief Handles own their entry, so they cannot be copied.
        stack_trace(const stack_trace&) = delete;
        /// @brief Handles own their entry, so they cannot be assigned.
        stack_trace& operator=(const stack_trace&) = delete;

        /// @brief Pops the entry pushed by `add`, if this handle owns one.
        inline ~stack_trace()
        {
            if (_owns)
            {
                _stack.pop_back();
            }
        }

        /// @brief Pushes `loc` onto the calling thread's call-site stack.
        /// @param loc Call-site location to record; defaults to the caller's location.
        /// @return A handle that owns the pushed entry.
        static inline stack_trace add(
            source_location loc = source_location::current())
        {
            _stack.emplace_back(hsd::move(loc));
            return stack_trace{true};
        }

        /// @brief Writes the calling thread's call-site stack to standard error.
        /// @details Entries are printed from most recently added to oldest.
        static inline void print_stack()
        {
            using namespace format_literals;

            for (const auto& _val : _stack | views::reverse)
            {
                println_err(
                    "Info: {}:{}\n\tFunction: {}"_fmt,
                    _val.file_name(), _val.line(), _val.function_name()
                );
            }
        }

        /// @brief Returns the most recently pushed call-site entry.
        /// @details Requires at least one entry on the calling thread's stack.
        /// @return Reference to the top of the stack.
        inline const auto& get() const
        {
            return _stack.back();
        }
    };

    /// @brief Error type that dumps the call-site stack before reporting failure.
    struct stack_trace_error
    {
        /// @brief Prints the current call-site stack, then returns a fixed message.
        /// @return A fixed explanatory string for use by `result`/`runtime_error` consumers.
        inline const char* pretty_error() const
        {
            stack_trace::print_stack();
            return "\nThe stack was unwind up there";
        }
    };
} // namespace hsd
