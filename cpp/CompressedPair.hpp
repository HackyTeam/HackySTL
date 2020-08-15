#pragma once

#include "Utility.hpp"

namespace hsd {
    namespace _detail {
        template <typename _Ty>
        struct _wrapper_for {
            _Ty value;
        };

        template <typename _Ty>
        using _wrap_type = std::conditional_t<std::is_class_v<_Ty>, _Ty, _wrapper_for<_Ty>>;

        template <typename _Ty>
        struct _is_empty : private _wrap_type<_Ty> {
            int a;
            inline static constexpr bool value = sizeof(int) == sizeof(_is_empty);
        };

        template <typename _Ty>
        using is_empty = std::bool_constant<_is_empty<_Ty>::value>;
    }

    template <typename _Ta, typename _Tb>
    class _compressed_pair_ab {
        _Ta a;
        _Tb b;
    public:
        template <typename _Aa, typename _Ab>
        _compressed_pair_ab(_Aa&& _a, _Ab&& _b)
            : a(forward<_Aa>(_a)), b(forward<_Ab>(_b)) {}

        constexpr _Ta& first() {
            return a;
        }
        constexpr _Ta const& first() const {
            return a;
        }
        constexpr _Tb& second() {
            return b;
        }
        constexpr _Tb const& second() const {
            return b;
        }

        constexpr _compressed_pair_ab() = default;
        constexpr _compressed_pair_ab(_compressed_pair_ab const&) = default;
        constexpr _compressed_pair_ab(_compressed_pair_ab&&) = default;
        constexpr _compressed_pair_ab& operator=(_compressed_pair_ab const&) = default;
        constexpr _compressed_pair_ab& operator=(_compressed_pair_ab&&) = default;
    };

    template <typename _Ta, typename _Tb>
    class _compressed_pair_b : private _Ta {
        _Tb b;
    public:
        template <typename _Aa, typename _Ab>
        _compressed_pair_b(_Aa&& _a, _Ab&& _b)
            : _Ta(forward<_Aa>(_a)), b(forward<_Ab>(_b)) {}

        constexpr _Ta& first() {
            return *this;
        }
        constexpr _Ta const& first() const {
            return *this;
        }
        constexpr _Tb& second() {
            return b;
        }
        constexpr _Tb const& second() const {
            return b;
        }

        constexpr _compressed_pair_b() = default;
        constexpr _compressed_pair_b(_compressed_pair_b const&) = default;
        constexpr _compressed_pair_b(_compressed_pair_b&&) = default;
        constexpr _compressed_pair_b& operator=(_compressed_pair_b const&) = default;
        constexpr _compressed_pair_b& operator=(_compressed_pair_b&&) = default;
    };

    template <typename _Ta, typename _Tb>
    class _compressed_pair_a : private _Tb {
        _Ta a;
    public:
        template <typename _Aa, typename _Ab>
        _compressed_pair_a(_Aa&& _a, _Ab&& _b)
            : _Tb(forward<_Ab>(_b)), a(forward<_Aa>(_a)) {}

        constexpr _Ta& first() {
            return a;
        }
        constexpr _Ta const& first() const {
            return a;
        }
        constexpr _Tb& second() {
            return *this;
        }
        constexpr _Tb const& second() const {
            return *this;
        }

        constexpr _compressed_pair_a() = default;
        constexpr _compressed_pair_a(_compressed_pair_a const&) = default;
        constexpr _compressed_pair_a(_compressed_pair_a&&) = default;
        constexpr _compressed_pair_a& operator=(_compressed_pair_a const&) = default;
        constexpr _compressed_pair_a& operator=(_compressed_pair_a&&) = default;
    };

    template <typename _Ta, typename _Tb>
    using _compressed_pair = std::conditional_t<
        _detail::_is_empty<_Ta>::value,
        _compressed_pair_b<_Ta, _Tb>,
        std::conditional_t<
            _detail::_is_empty<_Tb>::value,
            _compressed_pair_a<_Ta, _Tb>,
            _compressed_pair_ab<_Ta, _Tb>>
        >;

    template <size_t, typename _Base>
    struct get_helper
    {};
    
    template <typename _Base>
    struct get_helper<0, _Base>
    {
        static constexpr auto& get(_Base* value)
        {
            return value->first();
        }
    };

    template <typename _Base>
    struct get_helper<1, _Base>
    {
        static constexpr auto& get(_Base* value)
        {
            return value->second();
        }
    };

    template <typename _Ta, typename _Tb>
    class compressed_pair : public _compressed_pair<_Ta, _Tb> {
        using _Base = _compressed_pair<_Ta, _Tb>;

    public:
        using _Base::_Base;
        
        template <size_t _Idx>
        constexpr auto& get()
        {
            return get_helper<_Idx, _Base>::get(this);
        }

        template <size_t _Idx>
        constexpr auto& get() const
        {
            return get_helper<_Idx, _Base>::get(this);
        }
    };
}
