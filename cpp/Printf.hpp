#pragma once

#include "Tuparg.hpp"
#include "Result.hpp"

namespace hsd {
    Result<void, runtime_error> xvprintf(const char* fmt, xtuple_iterator tbegin, xtuple_iterator tend) noexcept;

    template <typename... Args>
    Result<void, runtime_error> xprintf(const char* fmt, Args&&... args) noexcept {
        auto targs = make_xtuple(forward<Args>(args)...);
        const auto& layout = xtuple_layout<decltype(targs)>::get_layout();
        return xvprintf(fmt, layout.begin(targs), layout.end());
    }
}
