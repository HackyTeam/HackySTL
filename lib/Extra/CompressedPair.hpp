#pragma once

/// @file CompressedPair.hpp
/// @brief Pair storage with empty-base optimization.
/// @details Keeps empty element types size-efficient while retaining pair-like
/// first/second and indexed access for ordinary and empty values.

#include "../Base/Utility.hpp"

namespace hsd
{
    namespace detail
    {
        template <typename _Ty>
        /// @brief Wrapper used when a non-class value cannot be an empty base.
        struct _wrapper_for
        {
            /// @brief Stored non-class value.
            _Ty value;
        };

        /// @brief Selects direct class storage or a value wrapper.
        template <typename _Ty>
        using _wrap_type = conditional_t<std::is_class_v<_Ty>, _Ty, _wrapper_for<_Ty>>;

        /// @brief Detects whether empty-base storage saves the expected space.
        template <typename _Ty>
        struct _is_empty : private _wrap_type<_Ty>
        {
            /// @brief Padding probe used to compare storage sizes.
            i32 a;
            /// @brief True when `_Ty` can be stored without an extra word.
            static constexpr bool value = sizeof(i32) == sizeof(_is_empty);
        };

        /// @brief Boolean constant exposing empty-storage detection.
        template <typename _Ty>
        using is_empty = literal_constant<bool, _is_empty<_Ty>::value>;
    } // namespace detail

    /// @brief Compressed pair layout storing both elements directly.
    template <typename _Ta, typename _Tb>
    class _compressed_pair_ab
    {
        /// @brief First stored element.
        _Ta a;
        /// @brief Second stored element.
        _Tb b;

    public:
        /// @brief Constructs both elements from forwarded values.
        /// @param _a First value forwarded into storage.
        /// @param _b Second value forwarded into storage.
        template <typename _Aa, typename _Ab>
        _compressed_pair_ab(_Aa&& _a, _Ab&& _b)
            : a(static_cast<_Ta>(hsd::forward<_Aa>(_a))), 
            b(static_cast<_Tb>(hsd::forward<_Ab>(_b)))
        {}

        /// @brief Returns mutable first-element access.
        /// @return Mutable reference to `a`.
        constexpr _Ta& first()
        {
            return a;
        }
        /// @brief Returns const first-element access.
        /// @return Const reference to `a`.
        constexpr _Ta const& first() const
        {
            return a;
        }
        /// @brief Returns mutable second-element access.
        /// @return Mutable reference to `b`.
        constexpr _Tb& second()
        {
            return b;
        }
        /// @brief Returns const second-element access.
        /// @return Const reference to `b`.
        constexpr _Tb const& second() const
        {
            return b;
        }

        /// @brief Default-constructs both elements.
        constexpr _compressed_pair_ab() = default;
        /// @brief Copies both elements.
        constexpr _compressed_pair_ab(_compressed_pair_ab const &) = default;
        /// @brief Moves both elements.
        constexpr _compressed_pair_ab(_compressed_pair_ab &&) = default;
        /// @brief Copy-assigns both elements.
        /// @return `*this` after assignment.
        constexpr _compressed_pair_ab& operator=(_compressed_pair_ab const &) = default;
        /// @brief Move-assigns both elements.
        /// @return `*this` after assignment.
        constexpr _compressed_pair_ab& operator=(_compressed_pair_ab &&) = default;
    };

    /// @brief Compressed pair layout using `_Ta` as an empty base.
    template <typename _Ta, typename _Tb>
    class _compressed_pair_b : private _Ta
    {
        /// @brief Second stored element; `_Ta` is inherited.
        _Tb b;

    public:
        /// @brief Constructs the empty base and second element.
        /// @param _a Value forwarded to the empty base.
        /// @param _b Value forwarded to the second element.
        template <typename _Aa, typename _Ab>
        _compressed_pair_b(_Aa&& _a, _Ab&& _b)
            : _Ta(hsd::forward<_Aa>(_a)), b(hsd::forward<_Ab>(_b)) {}

        /// @brief Returns mutable first-element access through the base.
        /// @return Mutable reference to the empty base.
        constexpr _Ta& first()
        {
            return *this;
        }
        /// @brief Returns const first-element access through the base.
        /// @return Const reference to the empty base.
        constexpr _Ta const& first() const
        {
            return *this;
        }
        /// @brief Returns mutable second-element access.
        /// @return Mutable reference to `b`.
        constexpr _Tb& second()
        {
            return b;
        }
        /// @brief Returns const second-element access.
        /// @return Const reference to `b`.
        constexpr _Tb const& second() const
        {
            return b;
        }

        /// @brief Default-constructs the compressed pair.
        constexpr _compressed_pair_b() = default;
        /// @brief Copies the compressed pair.
        constexpr _compressed_pair_b(_compressed_pair_b const&) = default;
        /// @brief Moves the compressed pair.
        constexpr _compressed_pair_b(_compressed_pair_b&&) = default;
        /// @brief Copy-assigns the compressed pair.
        /// @return `*this` after assignment.
        constexpr _compressed_pair_b& operator=(_compressed_pair_b const&) = default;
        /// @brief Move-assigns the compressed pair.
        /// @return `*this` after assignment.
        constexpr _compressed_pair_b& operator=(_compressed_pair_b&&) = default;
    };

    /// @brief Compressed pair layout using `_Tb` as an empty base.
    template <typename _Ta, typename _Tb>
    class _compressed_pair_a : private _Tb
    {
        /// @brief First stored element; `_Tb` is inherited.
        _Ta a;

    public:
        /// @brief Constructs the empty base and first element.
        /// @param _a Value forwarded to the first element.
        /// @param _b Value forwarded to the empty base.
        template <typename _Aa, typename _Ab>
        _compressed_pair_a(_Aa&& _a, _Ab&& _b)
            : _Tb(hsd::forward<_Ab>(_b)), a(hsd::forward<_Aa>(_a)) {}

        /// @brief Returns mutable first-element access.
        /// @return Mutable reference to `a`.
        constexpr _Ta& first()
        {
            return a;
        }
        /// @brief Returns const first-element access.
        /// @return Const reference to `a`.
        constexpr _Ta const& first() const
        {
            return a;
        }
        /// @brief Returns mutable second-element access through the base.
        /// @return Mutable reference to the empty base.
        constexpr _Tb& second()
        {
            return *this;
        }
        /// @brief Returns const second-element access through the base.
        /// @return Const reference to the empty base.
        constexpr _Tb const& second() const
        {
            return *this;
        }

        /// @brief Default-constructs the compressed pair.
        constexpr _compressed_pair_a() = default;
        /// @brief Copies the compressed pair.
        constexpr _compressed_pair_a(_compressed_pair_a const&) = default;
        /// @brief Moves the compressed pair.
        constexpr _compressed_pair_a(_compressed_pair_a&&) = default;
        /// @brief Copy-assigns the compressed pair.
        /// @return `*this` after assignment.
        constexpr _compressed_pair_a& operator=(_compressed_pair_a const&) = default;
        /// @brief Move-assigns the compressed pair.
        /// @return `*this` after assignment.
        constexpr _compressed_pair_a& operator=(_compressed_pair_a&&) = default;
    };

    /// @brief Selects the smallest compressed pair layout for two types.
    template <typename _Ta, typename _Tb>
    using _compressed_pair = conditional_t<
        detail::_is_empty<_Ta>::value,
        _compressed_pair_b<_Ta, _Tb>,
        conditional_t<
            detail::_is_empty<_Tb>::value,
            _compressed_pair_a<_Ta, _Tb>,
            _compressed_pair_ab<_Ta, _Tb>>>;

    /// @brief Indexed access dispatcher for compressed-pair bases.
    template <usize, typename _Base>
    struct get_helper
    {
        static constexpr auto& get(_Base* value) = delete;
    };

    /// @brief Dispatcher specialization for first-element access.
    template <typename _Base>
    struct get_helper<0, _Base>
    {
        /// @brief Returns the first element from a compressed-pair base.
        /// @param value Compressed-pair base pointer.
        /// @return Mutable first-element reference.
        static constexpr auto& get(_Base* value)
        {
            return value->first();
        }
    };

    /// @brief Dispatcher specialization for second-element access.
    template <typename _Base>
    struct get_helper<1, _Base>
    {
        /// @brief Returns the second element from a compressed-pair base.
        /// @param value Compressed-pair base pointer.
        /// @return Mutable second-element reference.
        static constexpr auto& get(_Base* value)
        {
            return value->second();
        }
    };

    /// @brief Public compressed pair with compile-time indexed access.
    template <typename _Ta, typename _Tb>
    class compressed_pair : public _compressed_pair<_Ta, _Tb>
    {
        /// @brief Selected compressed storage base.
        using _Base = _compressed_pair<_Ta, _Tb>;

    public:
        using _Base::_Base;

        template <usize _Idx>
        /// @brief Returns mutable access to element `_Idx`.
        /// @return Mutable reference to the selected element.
        constexpr auto& get()
        {
            return get_helper<_Idx, _Base>::get(this);
        }

        template <usize _Idx>
        /// @brief Returns const access to element `_Idx`.
        /// @return Const reference to the selected element.
        constexpr auto& get() const
        {
            return get_helper<_Idx, _Base>::get(this);
        }
    };
} // namespace hsd
