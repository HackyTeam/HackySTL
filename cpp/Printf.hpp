#pragma once

#include "Tuparg.hpp"

int xvprintf(const char* fmt, xtuple_iterator tbegin, xtuple_iterator tend);

template <typename... Args>
int xprintf(const char* fmt, Args&&... args) {
    auto targs = make_xtuple(std::forward<Args>(args)...);
    auto const& layout = xtuple_layout<decltype(targs)>::get_layout();
    return xvprintf(fmt, layout.begin(targs), layout.end());
}