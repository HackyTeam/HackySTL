#pragma once

#include <utility>
#include <cstdint>
#include <cstddef>
#include <cassert>

struct xtuple_iterator {
    size_t const* layout;
    void* addr;

    template <typename X>
    X& getnext() {
        assert(addr);

        X* p = reinterpret_cast<X*>(addr);
        addr = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(addr)
            + *layout
        );
        if (* ++layout == 0)
            addr = nullptr;
        return *p;
    }

    bool operator==(const xtuple_iterator& o) const {
        return addr == o.addr;
    }

    bool operator!=(const xtuple_iterator& o) const {
        return addr != o.addr;
    }
};

template <typename Tfirst, typename... Trest>
struct xtuple {
    Tfirst _first;
    xtuple<Trest...> _rest;

    xtuple(Tfirst f, Trest... r)
        : _first(std::move(f)), _rest(std::move(r)...)
    {}

    consteval static size_t get_offset() {
        return offsetof(xtuple, _rest);
    }
};

template <typename Tfirst>
struct xtuple<Tfirst> {
    Tfirst _first;

    xtuple(Tfirst f) : _first(std::move(f)) {}

    consteval static size_t get_offset() {
        return sizeof(xtuple);
    }
};

template <typename Tuple>
struct xtuple_layout;

template <typename Tfirst, typename... Trest>
struct xtuple_layout<xtuple<Tfirst, Trest...>> {
    using tuple_type = xtuple<Tfirst, Trest...>;
    constexpr static size_t count = 1 + sizeof...(Trest);

    size_t data[count + 1];

    consteval xtuple_layout() {
        assign_layout(data, 0);
        data[count] = 0;
    }

    consteval static void assign_layout(size_t* p, size_t idx) {
        p[idx] = tuple_type::get_offset();
        if constexpr (sizeof...(Trest))
            xtuple_layout<xtuple<Trest...>>::assign_layout(p, idx+1);
    }

    static inline xtuple_layout const& get_layout() {
        static constinit const xtuple_layout _layout;
        return _layout;
    }

    auto begin(tuple_type& tup) const { return xtuple_iterator{data, &tup._first}; }
    auto end() const { return xtuple_iterator{data + count, nullptr}; }
};

template <typename... Args>
auto make_xtuple(Args&&... args) {
    return xtuple<std::remove_reference_t<Args>...>(std::forward<Args&&>(args)...);
}
