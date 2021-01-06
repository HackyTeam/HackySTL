#pragma once

#include "Types.hpp"
#include "Result.hpp"
#include <cassert>

namespace hsd {
    struct xtuple_iterator {
        const usize* layout;
        void* addr;

        template <typename X>
        X& getnext_unsafe() {
            assert(addr);

            X* p = reinterpret_cast<X*>(addr);
            addr = reinterpret_cast<void*>(
                reinterpret_cast<uptr>(addr)
                + *layout
            );
            if (* ++layout == 0)
                addr = nullptr;
            return *p;
        }

        template <typename X>
        Result<reference<X>, runtime_error> getnext() {
            if (!addr)
                return runtime_error("Extracting value past end of the container");
            return Result<reference<X>, runtime_error>(getnext_unsafe<X>(), ok_tag_t{});
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
            : _first(move(f)), _rest(move(r)...)
        {}

        consteval static usize get_offset() {
            return offsetof(xtuple, _rest);
        }
    };

    template <typename Tfirst>
    struct xtuple<Tfirst> {
        Tfirst _first;

        xtuple(Tfirst f) : _first(move(f)) {}

        consteval static usize get_offset() {
            return sizeof(xtuple);
        }
    };

    template <typename Tuple>
    struct xtuple_layout;

    template <typename Tfirst, typename... Trest>
    struct xtuple_layout<xtuple<Tfirst, Trest...>> {
        using tuple_type = xtuple<Tfirst, Trest...>;
        constexpr static usize count = 1 + sizeof...(Trest);

        usize data[count + 1];

        consteval xtuple_layout() {
            assign_layout(data, 0);
            data[count] = 0;
        }

        consteval static void assign_layout(usize* p, usize idx) {
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
        return xtuple<remove_reference_t<Args>...>(forward<Args&&>(args)...);
    }
}
