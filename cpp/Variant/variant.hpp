#pragma once

#include <type_traits>
#include <exception>

namespace hsd {
// for private use
using std::move;
using std::forward;

// class monostate
struct monostate {};

constexpr bool operator==(monostate, monostate) noexcept {return true; }
constexpr bool operator>(monostate, monostate) noexcept {return false; }


//// helper classes
// class template variant_index
template <typename _Ty, size_t _Idx, typename _Tfirst, typename... _Trest>
struct _variant_index_helper : _variant_index_helper<_Ty, _Idx+1, _Trest...> {};

template <typename _Ty, size_t _Idx, typename... _Ts>
struct _variant_index_helper<_Ty, _Idx, _Ty, _Ts...> : std::integral_constant<size_t, _Idx> {};

template <typename _Ty, typename... _Ts>
struct _variant_index : _variant_index_helper<_Ty, 0, _Ts...> {};

// class bad_variant_access
struct bad_variant_access : public std::exception {
    virtual const char* what() const noexcept override {
        return "a variant was accessed with the wrong stored type";
    }
};

// forward declarations
template <typename _Ty>
struct variant_size;

template <size_t _Idx, typename _Ty>
struct variant_alternative;

template <size_t _Idx, typename... _Ts>
using variant_alternative_t = typename variant_alternative<_Idx, _Ts...>::type;


// class template variant
template <typename... _Ts>
struct variant {
    static_assert(sizeof...(_Ts) != 0, "empty variant is not allowed");
};

template <typename _Tfirst, typename... _Trest>
struct variant<_Tfirst, _Trest...> {
    constexpr variant() : _StoredIndex(0) {}

    constexpr variant(const variant& other)
        : _StoredIndex(other._StoredIndex), _Value(other._Value, _StoredIndex) {}
    constexpr variant(variant&& other)
        : _StoredIndex(other._StoredIndex), _Value(std::move(other._Value), _StoredIndex) {}

    template <typename _Tother, typename = std::enable_if_t<!std::is_same_v<variant, std::decay_t<_Tother>>>>
    constexpr variant(_Tother&& val)
        : _StoredIndex(_variant_index<typename _type_for<_Tother>::type, _Tfirst, _Trest...>::value),
            _Value(std::forward<_Tother>(val), _variant_index<typename _type_for<_Tother>::type, _Tfirst, _Trest...>()) {}

    ~variant() {
        _destroy();
    }

    constexpr variant& operator=(const variant& rhs) {
        _StorageTraits::assign_copy(_Value, rhs._Value, _StoredIndex, rhs.index());
        return *this;
    }
    constexpr variant& operator=(variant&& rhs) {
        _StorageTraits::assign_move(_Value, rhs._Value, _StoredIndex, rhs.index());
        return *this;
    }

    template <typename _Tother, typename = std::enable_if_t<!std::is_same_v<variant, std::decay_t<_Tother>>>>
    constexpr variant& operator=(_Tother&& rhs) {
        _StorageTraits::assign_fwd<_variant_index<typename _type_for<_Tother>::type, _Tfirst, _Trest...>::value>(_Value, _StoredIndex, std::forward<_Tother>(rhs));
        return *this;
    }

    constexpr size_t index() const noexcept {
        return _StoredIndex;
    }

private:
    void _destroy() {
        _storage_traits<decltype(_Value)>::destroy(_Value, _StoredIndex);
    }

    template <typename... _Ts>
    struct _select_overload_for {
        constexpr static void get();
    };

    template <typename _Tfirst, typename... _Trest>
    struct _select_overload_for<_Tfirst, _Trest...> : _select_overload_for<_Trest...> {
        using _select_overload_for<_Trest...>::get;
        constexpr static _Tfirst get(_Tfirst);
    };

    // Use variant passed template arguments
    using _select_overload = _select_overload_for<_Tfirst, _Trest...>;

    template <typename _Ty>
    struct _type_for {
        using type = decltype(_select_overload::get(std::declval<_Ty>()));
    };

    size_t _StoredIndex;

    struct _tag_first_member {};
    struct _tag_rest_member {};

    template <typename _Stor>
    struct _storage_traits;

    template <typename... _Ts>
    struct _variant_storage {
        constexpr _variant_storage(const _variant_storage&, size_t) {}
        constexpr _variant_storage(_variant_storage&&, size_t) {}
        template <typename _Ty>
        constexpr _variant_storage(_Ty&&, size_t) {}
    };

    template <typename _Tfirst, typename... _Trest>
    struct _variant_storage<_Tfirst, _Trest...> {
        union {
            _Tfirst _value_first;
            _variant_storage<_Trest...> _storage_rest;
        };

        using _RestTraits = _storage_traits<_variant_storage<_Trest...>>;

        // Default constructor
        constexpr _variant_storage() : _value_first() {}

        // Copy constructors
        constexpr _variant_storage(_tag_first_member, const _variant_storage& o)
            : _value_first(o._value_first) {}
        constexpr _variant_storage(_tag_rest_member, const _variant_storage& o, size_t id)
            : _storage_rest(o._storage_rest, id) {}
        constexpr _variant_storage(const _variant_storage& o, size_t id)
            : _variant_storage((id == 0)
                ? _variant_storage(_tag_first_member(), o)
                : _variant_storage(_tag_rest_member(), o, id-1)) {}

        // Move constructors
        constexpr _variant_storage(_tag_first_member, _variant_storage&& o)
            : _value_first(move(o._value_first)) {}
        constexpr _variant_storage(_tag_rest_member, _variant_storage&& o, size_t id)
            : _storage_rest(move(o._storage_rest), id) {}
        constexpr _variant_storage(_variant_storage&& o, size_t id)
            : _variant_storage((id == 0)
                ? _variant_storage(_tag_first_member(), move(o))
                : _variant_storage(_tag_rest_member(), move(o), id-1)) {}
        
        // Forwarding constructors
        template <typename _Ty>
        constexpr _variant_storage(_tag_first_member, _Ty&& val)
            : _value_first(forward<_Ty>(val)) {}
        template <typename _Ty, size_t _Idx>
        constexpr _variant_storage(_tag_rest_member, _Ty&& val, std::integral_constant<size_t, _Idx> id)
            : _storage_rest(forward<_Ty>(val), id) {}
        template <typename _Ty, size_t _Idx>
        constexpr _variant_storage(_Ty&& val, std::integral_constant<size_t, _Idx> id)
            : _variant_storage(this->_construct_index(forward<_Ty>(val), id)) {}
        
        template <typename _Ty, size_t _Idx>
        constexpr static _variant_storage _construct_index(_Ty&& val, std::integral_constant<size_t, _Idx>) {
            if constexpr (_Idx == 0)
                return _variant_storage(_tag_first_member(), forward<_Ty>(val));
            else
                return _variant_storage(_tag_rest_member(), forward<_Ty>(val), std::integral_constant<size_t, _Idx-1>());
        }
    };


    template <typename... _Ts>
    struct _storage_traits<_variant_storage<_Ts...>> {
        using Storage = _variant_storage<_Ts...>;
        static constexpr void destroy(Storage&, size_t) {}
        constexpr static bool equals_impl(size_t, Storage const&, Storage const&) {
            return false;
        }
        constexpr static void assign_copy_impl(Storage&, Storage const&, size_t) {}
        constexpr static void assign_move_impl(Storage&, Storage&, size_t) {}
    };

    template <typename _Tfirst, typename... _Trest>
    struct _storage_traits<_variant_storage<_Tfirst, _Trest...>> {
        using Storage = _variant_storage<_Tfirst, _Trest...>;
        // Out-of-place constructor
        constexpr static void construct_idx(Storage& l, Storage const& r, size_t id) {
            new (&l) Storage(r, id);
        }
        constexpr static void construct_idx(Storage& l, Storage&& r, size_t id) {
            new (&l) Storage(std::move(r), id);
        }
        template <size_t _Idx, typename _Ty>
        constexpr static void construct_fwd(Storage& l, _Ty&& value) {
            new (&l) Storage(std::forward<_Ty>(value), std::integral_constant<size_t, _Idx>());
        }
        // Destructor
        static void destroy(Storage& s, size_t id) {
            if (id == 0) {
                s._value_first.~_Tfirst();
            } else {
                Storage::_RestTraits::destroy(s._storage_rest, id-1);
            }
        }
        template <size_t _TheIdx>
        constexpr static auto& get_impl(Storage const& storage) {
            if constexpr (_TheIdx == 0) {
                return storage._value_first;
            } else {
                return Storage::_RestTraits::template get_impl<_TheIdx-1>(storage._storage_rest);
            }
        }
        constexpr static bool equals_impl(size_t id, Storage const& a, Storage const& b) {
            if (id == 0) {
                return a._value_first == b._value_first;
            } else {
                return Storage::_RestTraits::equals_impl(id-1, a._storage_rest, b._storage_rest);
            }
        }
        constexpr static void assign_copy_impl(Storage& l, Storage const& r, size_t id) {
            if (id == 0) {
                l._value_first = r._value_first;
            } else {
                Storage::_RestTraits::assign_copy_impl(l._storage_rest, r._storage_rest, id-1);
            }
        }
        constexpr static void assign_move_impl(Storage& l, Storage& r, size_t id) {
            if (id == 0) {
                l._value_first = std::move(r._value_first);
            } else {
                Storage::_RestTraits::assign_move_impl(l._storage_rest, r._storage_rest, id-1);
            }
        }
        constexpr static void assign_copy(Storage& l, Storage const& r, size_t& il, size_t ir) {
            if (il != ir) {
                destroy(l, il);
                il = ir;
                construct_idx(l, r, ir);
            }
            assign_copy_impl(l, r, ir);
        }
        constexpr static void assign_move(Storage& l, Storage& r, size_t& il, size_t ir) {
            if (il != ir) {
                destroy(l, il);
                il = ir;
                construct_idx(l, std::move(r), ir);
            }
            assign_move_impl(l, r, ir);
        }
        template <size_t _Idx, typename _Ty>
        constexpr static void assign_fwd_current(Storage& l, _Ty&& value) {
            if constexpr (_Idx == 0) {
                l._value_first = std::forward<_Ty>(value);
            } else {
                Storage::_RestTraits::assign_fwd_current(l._storage_rest, std::forward<_Ty>(value));
            }
        }
        template <size_t _Idx, typename _Ty>
        constexpr static void assign_fwd(Storage& l, size_t& il, _Ty&& value) {
            if (_Idx == il) {
                assign_fwd_current<_Idx>(l, std::forward<_Ty>(value));
            } else {
                destroy(l, il);
                il = _Idx;
                construct_fwd<_Idx>(l, std::forward<_Ty>(value));
            }
        }
    };

    _variant_storage<_Tfirst, _Trest...> _Value;
    using _StorageTraits = _storage_traits<_variant_storage<_Tfirst, _Trest...>>;
public:
    template <size_t _Idx>
    constexpr variant_alternative_t<_Idx, variant>& get() {
        return const_cast<variant_alternative_t<_Idx, variant>&>(const_cast<variant const*>(this)->get<_Idx>());
    }

    template <size_t _Idx>
    constexpr variant_alternative_t<_Idx, variant> const& get() const {
        if (index() != _Idx)
            throw bad_variant_access();
        return _StorageTraits::template get_impl<_Idx>(_Value);
    }

    template <typename _Ty>
    constexpr _Ty& get() {
        return const_cast<_Ty&>(const_cast<variant const*>(this)->get<_Ty>());
    }

    template <typename _Ty>
    constexpr _Ty const& get() const {
        return get<_variant_index<_Ty, _Tfirst, _Trest...>::value>();
    }


    template <size_t _Idx>
    constexpr variant_alternative_t<_Idx, variant>* get_if() noexcept {
        return const_cast<variant_alternative_t<_Idx, variant>*>(const_cast<variant const*>(this)->get_if<_Idx>());
    }

    template <size_t _Idx>
    constexpr variant_alternative_t<_Idx, variant> const* get_if() const noexcept {
        if (index() != _Idx)
            return nullptr;
        return &_StorageTraits::template get_impl<_Idx>(_Value);
    }

    template <typename _Ty>
    constexpr _Ty* get_if() noexcept {
        return const_cast<_Ty*>(const_cast<variant const*>(this)->get_if<_Ty>());
    }

    template <typename _Ty>
    constexpr _Ty const* get_if() const noexcept {
        return get_if<_variant_index<_Ty, _Tfirst, _Trest...>::value>();
    }


    // requires _Ty is a variant alternative
    template <typename _Ty>
    constexpr _Ty& emplace_one(_Ty&& value) {
        _destroy();
        _StoredIndex = _variant_index<std::remove_cv_t<std::remove_reference_t<_Ty>>, _Tfirst, _Trest...>::value;
        _StorageTraits::construct_fwd<_variant_index<std::remove_cv_t<std::remove_reference_t<_Ty>>, _Tfirst, _Trest...>::value>(_Value, std::forward<_Ty>(value));
    }


    template <typename _Ty>
    constexpr bool holds_alternative() const noexcept {
        return index() == _variant_index<_Ty, _Tfirst, _Trest...>::value;
    }

    friend constexpr bool operator==(variant const& a, variant const& b) {
        if (a.index() != b.index())
            return false;
        return _StorageTraits::equals_impl(a.index(), a._Value, b._Value);
    }

    friend constexpr bool operator!=(variant const& a, variant const& b) {
        return !(a == b);
    }
};

// class template variant_size
template <typename... _Ts>
struct variant_size<variant<_Ts...>> : std::integral_constant<size_t, sizeof...(_Ts)> {};

// class template variant_alternative
template <size_t _Idx, typename _Tfirst, typename... _Trest>
struct _variant_alternative_helper : _variant_alternative_helper<_Idx-1, _Trest...> {};

template <typename _Tfirst, typename... _Trest>
struct _variant_alternative_helper<0, _Tfirst, _Trest...> {
    using type = _Tfirst;
};

template <size_t _Idx, typename... _Ts>
struct variant_alternative<_Idx, variant<_Ts...>> : _variant_alternative_helper<_Idx, _Ts...> {};

//// Utility functions
// function template get
template <size_t _Idx, typename... _Ts>
constexpr variant_alternative_t<_Idx, variant<_Ts...>>& get(variant<_Ts...>& v) {
    return v.template get<_Idx>();
}

template <size_t _Idx, typename... _Ts>
constexpr variant_alternative_t<_Idx, variant<_Ts...>> const& get(variant<_Ts...> const& v) {
    return v.template get<_Idx>();
}

template <typename _Ty, typename... _Ts>
constexpr _Ty& get(variant<_Ts...>& v) {
    return v.template get<_Ty>();
}

template <typename _Ty, typename... _Ts>
constexpr _Ty const& get(variant<_Ts...> const& v) {
    return v.template get<_Ty>();
}

// function template get_if
template <size_t _Idx, typename... _Ts>
constexpr variant_alternative_t<_Idx, variant<_Ts...>>* get_if(variant<_Ts...>* v) noexcept {
    if (v == nullptr) return nullptr;
    return v->template get_if<_Idx>();
}

template <size_t _Idx, typename... _Ts>
constexpr variant_alternative_t<_Idx, variant<_Ts...>> const* get_if(variant<_Ts...> const* v) noexcept {
    if (v == nullptr) return nullptr;
    return v->template get_if<_Idx>();
}

template <typename _Ty, typename... _Ts>
constexpr _Ty* get_if(variant<_Ts...>* v) noexcept {
    if (v == nullptr) return nullptr;
    return v->template get_if<_Ty>();
}

template <typename _Ty, typename... _Ts>
constexpr _Ty const* get_if(variant<_Ts...> const* v) noexcept {
    if (v == nullptr) return nullptr;
    return v->template get_if<_Ty>();
}

// function template holds_alternative
template <typename _Ty, typename... _Ts>
constexpr bool holds_alternative(variant<_Ts...> const& v) noexcept {
    return v.template holds_alternative<_Ty>();
}

} // end namespace hsd
