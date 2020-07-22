#pragma once

#include <type_traits>

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
        _Value.destroy(_StoredIndex);
    }

    constexpr variant& operator=(const variant& rhs);
    constexpr variant& operator=(variant&& rhs);

    template <typename _Tother>
    constexpr variant& operator=(_Tother&& rhs);

    constexpr size_t index() const {
        return _StoredIndex;
    }

private:
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

    template <typename... _Ts>
    struct _variant_storage {
        constexpr _variant_storage(const _variant_storage&, size_t) {}
        constexpr _variant_storage(_variant_storage&&, size_t) {}
        template <typename _Ty>
        constexpr _variant_storage(_Ty&&, size_t) {}
        constexpr void destroy(size_t) {}
    };

    template <typename _Tfirst, typename... _Trest>
    struct _variant_storage<_Tfirst, _Trest...> {
        union {
            _Tfirst _value_first;
            _variant_storage<_Trest...> _storage_rest;
        };

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
        
        // Destructor
        void destroy(size_t id) {
            if (id == 0) {
                _value_first.~_Tfirst();
            } else {
                _storage_rest.destroy(id-1);
            }
        }
    };

    _variant_storage<_Tfirst, _Trest...> _Value;
};

// class template variant_size
template <typename _Ty>
struct variant_size;

template <typename... _Ts>
struct variant_size<variant<_Ts...>> : std::integral_constant<size_t, sizeof...(_Ts)> {};

// class template variant_alternative
template <size_t _Idx, typename _Tfirst, typename... _Trest>
struct _variant_alternative_helper : _variant_alternative_helper<_Idx-1, _Trest...> {};

template <typename _Tfirst, typename... _Trest>
struct _variant_alternative_helper<0, _Tfirst, _Trest...> {
    using type = _Tfirst;
};

template <size_t _Idx, typename _Ty>
struct variant_alternative;

template <size_t _Idx, typename... _Ts>
struct variant_alternative<_Idx, variant<_Ts...>> : _variant_alternative_helper<_Idx, _Ts...> {};

} // end namespace hsd