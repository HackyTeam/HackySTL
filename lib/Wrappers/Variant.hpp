#pragma once

/// @file Variant.hpp
/// @brief Tagged heterogeneous storage with checked access and visitation.
/// @details Stores one alternative at a time, tracks its index, supports
/// type/index retrieval, in-place construction, assignment, and visit calls.

#include "../Base/Result.hpp"
#include "Reference.hpp"

namespace hsd
{
    /// @brief Empty alternative used for valueless/default variant storage.
    struct monostate {};

    /// @brief Equality for `monostate`: always equal, since it carries no state.
    /// @return Always `true`.
    constexpr bool operator==(monostate, monostate) noexcept
    {
        return true;
    }

    /// @brief Implementation details backing `variant`: alternative storage,
    /// visitor return-type deduction, and recursive dispatch helpers.
    namespace variant_detail
    {
        /// @brief Single alternative slot pairing a compile-time index with a value.
        /// @tparam Idx Index of this alternative within the owning variant's type list.
        /// @tparam T Alternative type held by this slot.
        template <usize Idx, typename T>
        struct internal_storage
        {
            /// @brief Index of this alternative within the owning variant's type list.
            static constexpr usize index = Idx;
            T value; ///< The stored alternative value.

            /// @brief Value-initialize `value`; the active alternative is
            /// established separately via placement construction. A default
            /// construction value-initializes `value`, so a default-constructed
            /// variant holds `T{}` rather than an indeterminate value.
            constexpr internal_storage() : value{} {}

            /// @brief Construct `value` in place by forwarding arguments to `T`.
            /// @param args Arguments forwarded to `T`'s constructor.
            template <typename... Args>
            constexpr internal_storage(Args&&... args)
                : value{hsd::forward<Args>(args)...}
            {}

            /// @brief No-op; the active alternative is destroyed explicitly by
            /// the owning `storage`/`variant` before this scope ends.
            constexpr ~internal_storage() {}
        };

        /// @brief Deduces the return type of invoking `Func` with `Arg`.
        template <typename Func, typename Arg>
        using invoke_return_type = decltype(declval<Func>()(declval<Arg>()));

        /// @brief Computes the value type produced by `variant::visit()`.
        /// @details Reference-returning visitors are wrapped in `hsd::reference`
        /// so the result can be stored by value in `result`/`option_err`.
        /// @tparam Func Visitor callable type.
        /// @tparam Arg Alternative type passed to the visitor.
        template <typename Func, typename Arg>
        struct visit_return_type
        {
            /// @brief Raw (possibly reference) type returned by invoking `Func`.
            using invoke_ret = invoke_return_type<Func, Arg>;

            /// @brief `invoke_ret`, or `reference<remove_reference_t<invoke_ret>>`
            /// if `invoke_ret` is a reference type.
            using type = conditional_t<
                IsReference<invoke_ret>,
                reference<remove_reference_t<invoke_ret>>,
                invoke_ret
            >;
        };

        /// @brief Sentinel type produced by `type_at`/`constructible_at` when no
        /// alternative matches; never a valid, constructible alternative.
        class error_placeholder;

        /// @brief Recursive alternative storage for a variant's type list.
        /// @tparam Idx Index of the first alternative in this instantiation.
        /// @tparam T Alternative types, in order, starting at `Idx`.
        template <usize Idx, typename... T>
        class storage;

        /// @brief Storage specialization for the last alternative in the list.
        /// @details Terminates the recursive `storage` chain; holds the final
        /// alternative directly instead of delegating to a nested `RestType`.
        template <usize Idx, typename T>
        class storage<Idx, T>
        {
        private:
            /// @brief Exactly one of these is active: the alternative once
            /// constructed, or a placeholder before any alternative exists.
            union {
                internal_storage<Idx, T> _first; ///< The alternative, once constructed.
                monostate _second{}; ///< Placeholder while no alternative has been constructed.
            };

        public:
            /// @brief `T` if `TypeIdx == Idx`, otherwise `error_placeholder`.
            template <usize TypeIdx>
            using type_at = hsd::conditional_t<Idx == TypeIdx, T, error_placeholder>;

            /// @brief `T` if constructible from `U`, otherwise `error_placeholder`.
            template <typename U>
            using constructible_at = hsd::conditional_t<Constructible<T, U>, T, error_placeholder>;

            /// @brief Leave the union inactive (`_second`); no alternative is constructed yet.
            constexpr storage()
                : _second{}
            {}
            
            /// @brief Not copyable/movable directly; `variant` manages
            /// alternative construction via construct()/destroy() instead.
            constexpr storage(const storage&) = delete;
            constexpr storage(storage&&) = delete;

            /// @brief No-op; the caller must destroy the active alternative
            /// via destroy() before this storage is destroyed.
            constexpr ~storage() {}

            /// @brief Not copyable/movable directly; `variant` manages
            /// alternative assignment via get_value()/construct() instead.
            constexpr storage& operator=(const storage&) = delete;
            constexpr storage& operator=(storage&&) = delete;

            /// @brief Invoke `func` with the stored value if `idx` matches this slot.
            /// @details Terminal case of the recursive dispatch; there is no
            /// nested `RestType` to fall back to here.
            /// @param idx Active alternative index to match against `Idx`.
            /// @param func Visitor callable invoked with an rvalue reference to the value.
            /// @return Wrapped visitor result on match, or a `runtime_error` if `idx != Idx`.
            template <typename Func> requires (Invocable<Func, T>)
            constexpr auto visit(usize idx, Func&& func)
            {
                using InvokeRetType = typename visit_return_type<Func, T>::type;

                if constexpr (IsSame<InvokeRetType, void>)
                {
                    using RetType = option_err<runtime_error>;

                    if (idx != Idx) [[unlikely]]
                    {
                        return RetType{
                            runtime_error{"Index is out of range"}
                        };
                    }

                    func(hsd::forward<T>(_first.value));
                    return RetType{};
                }
                else
                {
                    using RetType = result<typename visit_return_type<Func, T>::type, runtime_error>;

                    if (idx != Idx) [[unlikely]]
                    {
                        return RetType{
                            runtime_error{"Index is out of range"}
                        };
                    }

                    return RetType{
                        InvokeRetType{func(hsd::forward<T>(_first.value))}
                    };
                }
            }

            /// @brief Const-qualified overload of visit(usize, Func&&).
            /// @param idx Active alternative index to match against `Idx`.
            /// @param func Visitor callable invoked with a const rvalue reference to the value.
            /// @return Wrapped visitor result on match, or a `runtime_error` if `idx != Idx`.
            template <typename Func> requires (Invocable<Func, T>)
            constexpr auto visit(usize idx, Func&& func) const
            {
                using InvokeRetType = typename visit_return_type<Func, T>::type;

                if constexpr (IsSame<InvokeRetType, void>)
                {
                    using RetType = option_err<runtime_error>;

                    if (idx != Idx) [[unlikely]]
                    {
                        return RetType{
                            runtime_error{"Index is out of range"}
                        };
                    }

                    func(hsd::forward<const T>(_first.value));
                    return RetType{};
                }
                else
                {
                    using RetType = result<typename visit_return_type<Func, T>::type, runtime_error>;

                    if (idx != Idx) [[unlikely]]
                    {
                        return RetType{
                            runtime_error{"Index is out of range"}
                        };
                    }

                    return RetType{
                        InvokeRetType{func(hsd::forward<const T>(_first.value))}
                    };
                }
            }

            /// @brief Destroy the stored value if `idx` matches this slot.
            /// @param idx Active alternative index to match against `Idx`.
            /// @return Empty on success, or a `runtime_error` if `idx != Idx`.
            constexpr option_err<runtime_error> destroy(usize idx)
            {
                if (idx != Idx) [[unlikely]]
                {
                    return runtime_error{"Index is out of range"};
                }

                _first.value.~T();
                return {};
            }

            /// @brief Construct the alternative in place. Does not check that
            /// `idx` currently designates it; the caller must already have
            /// updated the active index before calling this.
            /// @tparam U Alternative type to construct; must equal `T`.
            /// @param args Arguments forwarded to `T`'s constructor.
            template <typename U, typename... Args>
            requires (Constructible<T, Args...> && hsd::IsSame<T, U>)
            constexpr void construct(Args&&... args)
            {
                hsd::construct_at(&_first, hsd::forward<Args>(args)...);
            }

            /// @brief Access the stored value by alternative type.
            /// @tparam U Alternative type to access; must equal `T`.
            /// @return Reference to the stored value. Undefined behavior if
            /// this slot is not the active alternative.
            template <typename U>
            requires (hsd::IsSame<T, U>)
            constexpr auto& get_value()
            {
                return _first.value;
            }

            /// @brief Const-qualified overload of get_value<U>().
            /// @return Const reference to the stored value.
            template <typename U>
            requires (hsd::IsSame<T, U>)
            constexpr const auto& get_value() const
            {
                return _first.value;
            }

            /// @brief Access the stored value by alternative index.
            /// @tparam ValIdx Alternative index to access; must equal `Idx`.
            /// @return Reference to the stored value. Undefined behavior if
            /// this slot is not the active alternative.
            template <usize ValIdx>
            requires (Idx == ValIdx)
            constexpr auto& get_value()
            {
                return _first.value;
            }

            /// @brief Const-qualified overload of get_value<ValIdx>().
            /// @return Const reference to the stored value.
            template <usize ValIdx>
            requires (Idx == ValIdx)
            constexpr const auto& get_value() const
            {
                return _first.value;
            }

            /// @brief Compute the index of alternative `U` within this storage.
            /// @tparam U Alternative type to look up; must equal `T`.
            /// @return `Idx`.
            template <typename U>
            requires (hsd::IsSame<T, U>)
            static consteval usize get_index()
            {
                return Idx;
            }
        };

        /// @brief Storage specialization for a non-final alternative.
        /// @details Recursively delegates operations for other indices to a
        /// nested `RestType` storage, forming a chain that holds exactly one
        /// active alternative at a time via a shared union.
        /// @tparam Idx Index of the first alternative (`T`) in this instantiation.
        /// @tparam T Alternative type held directly by this level.
        /// @tparam Rest Remaining alternative types, delegated to `RestType`.
        template <usize Idx, typename T, typename... Rest>
        class storage<Idx, T, Rest...>
        {
        private:
            /// @brief Storage for the remaining alternatives, starting at `Idx + 1`.
            using RestType = storage<Idx + 1, Rest...>;

            /// @brief Exactly one of these is active: this level's
            /// alternative, or the nested storage for a later alternative.
            union {
                internal_storage<Idx, T> _first; ///< This level's alternative, once constructed.
                RestType _rest{}; ///< Storage for a later alternative, when active.
            };

        public:
            /// @brief `T` if `TypeIdx == Idx`, otherwise `RestType::type_at<TypeIdx>`.
            template <usize TypeIdx>
            using type_at = conditional_t<Idx == TypeIdx, T, typename RestType::template type_at<TypeIdx>>;

            /// @brief `T` if constructible from `U`, otherwise
            /// `RestType::constructible_at<U>`.
            template <typename U>
            using constructible_at = conditional_t<
                Constructible<T, U>, T, typename RestType::template constructible_at<U>
            >;

            /// @brief Leave the union inactive (delegating to `_rest`'s
            /// default state); no alternative is constructed yet.
            constexpr storage()
                : _rest{}
            {}
            
            /// @brief Not copyable/movable directly; `variant` manages
            /// alternative construction via construct()/destroy() instead.
            constexpr storage(const storage&) = delete;
            constexpr storage(storage&&) = delete;

            /// @brief No-op; the caller must destroy the active alternative
            /// via destroy() before this storage is destroyed.
            constexpr ~storage() {}

            /// @brief Not copyable/movable directly; `variant` manages
            /// alternative assignment via get_value()/construct() instead.
            constexpr storage& operator=(const storage&) = delete;
            constexpr storage& operator=(storage&&) = delete;

            /// @brief Invoke `func` with the stored value at `idx`, recursing
            /// into `_rest` when `idx` does not designate this level's alternative.
            /// @param idx Active alternative index.
            /// @param func Visitor callable invoked with an rvalue reference to the value.
            /// @return Wrapped visitor result, or a `runtime_error` if no
            /// alternative in the chain matches `idx`.
            template <typename Func> requires (Invocable<Func, T>)
            constexpr auto visit(usize idx, Func&& func)
            {
                if (idx != Idx)
                {
                    return _rest.visit(idx, hsd::forward<Func>(func));
                }

                using InvokeRetType = typename visit_return_type<Func, T>::type;

                if constexpr (IsSame<InvokeRetType, void>)
                {
                    using RetType = option_err<runtime_error>;
                    func(hsd::forward<T>(_first.value));
                    return RetType{};
                }
                else
                {
                    using RetType = result<typename visit_return_type<Func, T>::type, runtime_error>;
                    return RetType{
                        InvokeRetType{func(hsd::forward<T>(_first.value))}
                    };
                }
            }

            /// @brief Const-qualified overload of visit(usize, Func&&).
            /// @param idx Active alternative index.
            /// @param func Visitor callable invoked with a const rvalue reference to the value.
            /// @return Wrapped visitor result, or a `runtime_error` if no
            /// alternative in the chain matches `idx`.
            template <typename Func> requires (Invocable<Func, T>)
            constexpr auto visit(usize idx, Func&& func) const
            {
                if (idx != Idx)
                {
                    return _rest.visit(idx, hsd::forward<Func>(func));
                }

                using InvokeRetType = typename visit_return_type<Func, T>::type;

                if constexpr (IsSame<InvokeRetType, void>)
                {
                    using RetType = option_err<runtime_error>;
                    func(hsd::forward<const T>(_first.value));
                    return RetType{};
                }
                else
                {
                    using RetType = result<typename visit_return_type<Func, T>::type, runtime_error>;
                    return RetType{
                        InvokeRetType{func(hsd::forward<const T>(_first.value))}
                    };
                }
            }

            /// @brief Destroy the stored value at `idx`, recursing into
            /// `_rest` when `idx` does not designate this level's alternative.
            /// @param idx Active alternative index.
            /// @return Empty on success, or a `runtime_error` if no
            /// alternative in the chain matches `idx`.
            constexpr option_err<runtime_error> destroy(usize idx)
            {
                if (idx != Idx)
                {
                    return _rest.destroy(idx);
                }

                _first.value.~T();
                return {};
            }

            /// @brief Construct alternative `U` in place, recursing into
            /// `_rest` when `U` is not this level's alternative type. Does not
            /// check that the active index has been updated; the caller must
            /// do so before calling this.
            /// @tparam U Alternative type to construct.
            /// @param args Arguments forwarded to `U`'s constructor.
            template <typename U, typename... Args>
            constexpr void construct(Args&&... args)
            {
                if constexpr (!hsd::IsSame<T, U>)
                {
                    _rest.template construct<U>(hsd::forward<Args>(args)...);
                }
                else
                {
                    static_assert(Constructible<T, Args...>);
                    hsd::construct_at(&_first, hsd::forward<Args>(args)...);
                }
            }

            /// @brief Access the stored value by alternative type, recursing
            /// into `_rest` when `U` is not this level's alternative type.
            /// @tparam U Alternative type to access.
            /// @return Reference to the stored value. Undefined behavior if
            /// no slot in the chain matching `U` is the active alternative.
            template <typename U>
            constexpr auto& get_value()
            {
                if constexpr (!IsSame<U, T>)
                {
                    return _rest.template get_value<U>();
                }
                else
                {
                    return _first.value;
                }
            }

            /// @brief Const-qualified overload of get_value<U>().
            /// @return Const reference to the stored value.
            template <typename U>
            constexpr const auto& get_value() const
            {
                if constexpr (!IsSame<U, T>)
                {
                    return _rest.template get_value<U>();
                }
                else
                {
                    return _first.value;
                }
            }

            /// @brief Access the stored value by alternative index, recursing
            /// into `_rest` when `ValIdx != Idx`.
            /// @tparam ValIdx Alternative index to access.
            /// @return Reference to the stored value. Undefined behavior if
            /// no slot in the chain at `ValIdx` is the active alternative.
            template <usize ValIdx>
            constexpr auto& get_value()
            {
                if constexpr (ValIdx != Idx)
                {
                    return _rest.template get_value<ValIdx>();
                }
                else
                {
                    return _first.value;
                }
            }

            /// @brief Const-qualified overload of get_value<ValIdx>().
            /// @return Const reference to the stored value.
            template <usize ValIdx>
            constexpr const auto& get_value() const
            {
                if constexpr (ValIdx != Idx)
                {
                    return _rest.template get_value<ValIdx>();
                }
                else
                {
                    return _first.value;
                }
            }

            /// @brief Compute the index of alternative `U`, recursing into
            /// `RestType` when `U` is not this level's alternative type.
            /// @tparam U Alternative type to look up.
            /// @return Index of `U` within the full alternative list.
            template <typename U>
            static consteval usize get_index()
            {
                if constexpr (!hsd::IsSame<T, U>)
                {
                    return RestType::template get_index<U>();
                }
                else
                {
                    return Idx;
                }
            }
        };
    }

    /// @brief Tag selecting direct construction of a variant alternative.
    /// @tparam T Alternative type to construct directly.
    template <typename T>
    struct in_place_t {};

    /// @brief Tagged union with checked type/index access and visitation.
    /// @tparam Ts Alternative types the variant can hold.
    template <typename... Ts>
    class variant
    {
    public:
        /// @brief Sentinel index() value: no alternative is currently active
        /// (only possible when the variant was default-constructed and no
        /// `Ts` is default-constructible).
        static constexpr usize npos = static_cast<usize>(-1);

    private:
        /// @brief Recursive alternative storage for the full type list `Ts...`.
        using storage_type = variant_detail::storage<0, Ts...>;

        usize _stored_index = npos; ///< Index of the active alternative, or `npos` if none is constructed.
        storage_type _storage = {}; ///< Union-based storage holding the active alternative's value.

        /// @brief Copy-construct the active alternative from `val` into a
        /// freshly reset `_storage`.
        /// @details Requires `_stored_index == val._stored_index` (the
        /// destination alternative type). `_storage` must not currently hold
        /// a live alternative, since it is placement-reconstructed before the
        /// copy. Called from the copy constructor and from copy assignment
        /// when switching to a different active alternative.
        /// @param val Variant to copy the active alternative from.
        constexpr void _construct_from_variant(const variant& val)
        {
            hsd::construct_at(&_storage);

            val._storage.visit(
                _stored_index,
                [this]<typename T>(const T& val) {
                    _storage.template construct<T>(val);
                }
            ).unwrap();
        }

        /// @brief Move-construct the active alternative from `val` into a
        /// freshly reset `_storage`.
        /// @details Same preconditions as _construct_from_variant(const
        /// variant&), but moves rather than copies the source value. Does
        /// not clear `val`'s active index; `val` remains responsible for
        /// destroying its (now moved-from) value.
        /// @param val Variant to move the active alternative from.
        constexpr void _construct_from_variant(variant&& val)
        {
            hsd::construct_at(&_storage);

            val._storage.visit(
                _stored_index,
                [this]<typename T>(T&& val) {
                    _storage.template construct<T>(hsd::move(val));
                }
            ).unwrap();
        }

        /// @brief Copy-assign the active alternative from `val` into the
        /// already-constructed alternative at the same index.
        /// @details Requires `_stored_index == val._stored_index`; used when
        /// both variants already hold the same alternative, so no
        /// construct()/destroy() cycle is needed.
        /// @param val Variant to copy the active alternative's value from.
        constexpr void _attribute_from_variant(const variant& val)
        {
            val._storage.visit(
                _stored_index,
                [this]<typename T>(const T& val) {
                    _storage.template get_value<T>() = val;
                }
            ).unwrap();
        }

        /// @brief Move-assign the active alternative from `val` into the
        /// already-constructed alternative at the same index.
        /// @details Same precondition as _attribute_from_variant(const
        /// variant&), but moves rather than copies the source value.
        /// @param val Variant to move the active alternative's value from.
        constexpr void _attribute_from_variant(variant&& val)
        {
            val._storage.visit(
                _stored_index,
                [this]<typename T>(T&& val) {
                    _storage.template get_value<T>() = hsd::move(val);
                }
            ).unwrap();
        }

        /// @brief Destroy the active alternative, if any.
        /// @details No-op when `_stored_index == npos`. Does not reset
        /// `_stored_index`; callers that leave the variant valueless must do
        /// so explicitly.
        constexpr void _destroy_variant()
        {
            if (_stored_index != npos)
            {
                _storage.destroy(_stored_index).unwrap();
            }
        }

        /// @brief Replace the active alternative with a value of type `T`.
        /// @details If `T` is already the active alternative, assigns into it
        /// in place; otherwise destroys the current alternative (if any) and
        /// placement-constructs `T` from `rhs`, updating `_stored_index`.
        /// @tparam T Alternative type to become active.
        /// @tparam U Forwarding-reference type of the assigned value.
        /// @param rhs Value assigned or used to construct the new alternative.
        template <typename T, typename U>
        constexpr void _attribute_to(U&& rhs)
        {
            if (_stored_index == storage_type::template get_index<T>())
            {
                _storage.template get_value<T>() = hsd::forward<U>(rhs);
            }
            else
            {
                _destroy_variant();
                hsd::construct_at(&_storage);
                _stored_index = storage_type::template get_index<T>();
                _storage.template construct<T>(hsd::forward<U>(rhs));
            }
        }

    public:
        /// @brief Default-construct the first default-constructible alternative.
        /// @details Tries each `Ts` in order and constructs the first one
        /// that satisfies `DefaultConstructible`. If none does, the variant
        /// is left valueless (`_stored_index == npos`, no alternative constructed).
        constexpr variant()
        {
            auto _construct_func = [this]<typename T>
            {
                if constexpr (DefaultConstructible<T>)
                {
                    _stored_index = storage_type::template get_index<T>();
                    _storage.template construct<T>();
                    return true;
                }
                else
                {
                    return false;
                }
            };

            ((_construct_func.template operator()<Ts>()) || ...);
        }

        /// @brief Destroy the active alternative, if any.
        constexpr ~variant()
        {
            _destroy_variant();
        }

        /// @brief Construct from a value convertible to one of `Ts`.
        /// @details Excluded when `T` is (cv/ref-qualified) `variant` itself,
        /// so this does not compete with the copy/move constructors. Prefers
        /// the alternative exactly matching `remove_cvref_t<T>`; otherwise
        /// selects the first alternative constructible from `T`.
        /// @tparam T Type of the value used to construct an alternative.
        /// @param val Value forwarded into the selected alternative's constructor.
        template <typename T>
        requires (!IsSame<remove_cvref_t<T>, variant<Ts...>>)
        constexpr variant(T&& val)
        {
            // The alternative is the type that `constructible_at` selects; `T` may be an
            // lvalue reference, which is not an alternative and must not be looked up.
            using DesiredType = typename storage_type::template constructible_at<T>;

            _stored_index = storage_type::template get_index<DesiredType>();
            _storage.template construct<DesiredType>(hsd::forward<T>(val));
        }

        /// @brief Construct alternative `T` directly, forwarding `args` to its constructor.
        /// @tparam T Alternative type to construct.
        /// @param args Arguments forwarded to `T`'s constructor.
        template <typename T, typename... Args>
        constexpr variant(in_place_t<T>, Args&&... args)
            : _stored_index{storage_type::template get_index<T>()}
        {
            _storage.template construct<T>(hsd::forward<Args>(args)...);
        }

        /// @brief Copy-construct, replicating the active alternative from `val`.
        /// @param val Variant to copy.
        constexpr variant(const variant& val)
            : _stored_index{val._stored_index}
        {
            _construct_from_variant(val);
        }

        /// @brief Move-construct, moving the active alternative from `val`.
        /// @details `val` is left with its original active index and a
        /// moved-from value; it is not made valueless.
        /// @param val Variant to move from.
        constexpr variant(variant&& val)
            : _stored_index{val._stored_index}
        {
            _construct_from_variant(hsd::move(val));
        }

        /// @brief Assign a value convertible to one of `Ts`, becoming the active alternative.
        /// @details Selection follows the same rules as the converting
        /// constructor. Assigns in place if the target alternative is already
        /// active, otherwise destroys the current alternative and constructs
        /// the new one.
        /// @tparam T Type of the assigned value.
        /// @param rhs Value forwarded into the selected alternative.
        /// @return Reference to this variant.
        template <typename T>
        requires (!IsSame<remove_cvref_t<T>, variant<Ts...>>)
        constexpr variant& operator=(T&& rhs)
        {
            using DesiredType = typename storage_type::template constructible_at<T>;

            if constexpr (IsSame<DesiredType, remove_cvref_t<T>>)
            {
                _attribute_to<T>(hsd::forward<T>(rhs));
            }
            else
            {
                _attribute_to<DesiredType>(hsd::forward<T>(rhs));
            }

            return *this;
        }

        /// @brief Copy-assign from another variant.
        /// @details Assigns into the current alternative in place if both
        /// variants already hold the same active index; otherwise destroys
        /// the current alternative and copy-constructs the new one.
        /// @param rhs Variant to copy from.
        /// @return Reference to this variant.
        constexpr variant& operator=(const variant& rhs)
        {
            if (_stored_index == rhs._stored_index)
            {
                _attribute_from_variant(rhs);
            }
            else
            {
                _destroy_variant();
                _stored_index = rhs._stored_index;
                _construct_from_variant(rhs);
            }

            return *this;
        }

        /// @brief Move-assign from another variant.
        /// @details Same dispatch as copy assignment, but moves rather than
        /// copies the source value. `rhs` is left with its original active
        /// index and a moved-from value.
        /// @param rhs Variant to move from.
        /// @return Reference to this variant.
        constexpr variant& operator=(variant&& rhs)
        {
            if (_stored_index == rhs._stored_index)
            {
                _attribute_from_variant(hsd::move(rhs));
            }
            else
            {
                _destroy_variant();
                _stored_index = rhs._stored_index;
                _construct_from_variant(hsd::move(rhs));
            }

            return *this;
        }

        /// @brief Access the active alternative by type.
        /// @tparam T Alternative type to retrieve.
        /// @return A reference wrapper to the value on success, or a
        /// `runtime_error` if `T` is not the active alternative.
        template <typename T>
        constexpr auto get()
            -> result<reference<T>, runtime_error>
        {
            if (_stored_index != storage_type::template get_index<T>())
            {
                return runtime_error{"Variant does not hold this type"};
            }

            return reference<T>{_storage.template get_value<T>()};
        }

        /// @brief Const-qualified overload of get<T>().
        /// @return A reference wrapper to the const value on success, or a
        /// `runtime_error` if `T` is not the active alternative.
        template <typename T>
        constexpr auto get() const
            -> result<reference<const T>, runtime_error>
        {
            if (_stored_index != storage_type::template get_index<T>())
            {
                return runtime_error{"Variant does not hold this type"};
            }

            return reference<const T>{_storage.template get_value<T>()};
        }

        /// @brief Access the active alternative by type without error wrapping.
        /// @tparam T Alternative type to retrieve.
        /// @return Pointer to the value, or `nullptr` if `T` is not the active alternative.
        template <typename T>
        constexpr T* get_if()
        {
            if (_stored_index != storage_type::template get_index<T>())
            {
                return nullptr;
            }

            return hsd::addressof(_storage.template get_value<T>());
        }

        /// @brief Const-qualified overload of get_if<T>().
        /// @return Const pointer to the value, or `nullptr` if `T` is not the active alternative.
        template <typename T>
        constexpr const T* get_if() const
        {
            if (_stored_index != storage_type::template get_index<T>())
            {
                return nullptr;
            }

            return hsd::addressof(_storage.template get_value<T>());
        }

        /// @brief Access the active alternative by index.
        /// @tparam Idx Alternative index to retrieve.
        /// @return A reference wrapper to the value on success, or a
        /// `runtime_error` if `Idx` is not the active index.
        template <usize Idx>
        constexpr auto get()
            -> result<reference<typename storage_type::template type_at<Idx>>, runtime_error>
        {
            if (_stored_index != Idx)
            {
                return runtime_error{"Variant does not hold this type"};
            }

            using ValueType = typename storage_type::template type_at<Idx>;
            return reference<ValueType>{_storage.template get_value<Idx>()};
        }

        /// @brief Const-qualified overload of get<Idx>().
        /// @return A reference wrapper to the const value on success, or a
        /// `runtime_error` if `Idx` is not the active index.
        template <usize Idx>
        constexpr auto get() const
            -> result<reference<const typename storage_type::template type_at<Idx>>, runtime_error>
        {
            if (_stored_index != Idx)
            {
                return runtime_error{"Variant does not hold this type"};
            }

            using ValueType = typename storage_type::template type_at<Idx>;
            return reference<const ValueType>{_storage.template get_value<Idx>()};
        }

        /// @brief Access the active alternative by index without error wrapping.
        /// @tparam Idx Alternative index to retrieve.
        /// @return Pointer to the value, or `nullptr` if `Idx` is not the active index.
        template <usize Idx>
        constexpr auto get_if()
            -> typename storage_type::template type_at<Idx>*
        {
            if (_stored_index != Idx)
            {
                return nullptr;
            }

            return hsd::addressof(_storage.template get_value<Idx>());
        }

        /// @brief Const-qualified overload of get_if<Idx>().
        /// @return Const pointer to the value, or `nullptr` if `Idx` is not the active index.
        template <usize Idx>
        constexpr auto get_if() const
            -> const typename storage_type::template type_at<Idx>*
        {
            if (_stored_index != Idx)
            {
                return nullptr;
            }

            return hsd::addressof(_storage.template get_value<Idx>());
        }

        /// @brief Check whether `T` is the currently active alternative.
        /// @tparam T Alternative type to test.
        /// @return `true` if `T` is active, `false` otherwise.
        template <typename T>
        constexpr bool holds() const
        {
            return _stored_index == storage_type::template get_index<T>();
        }

        /// @brief Invoke `func` with the active alternative's value.
        /// @tparam Func Visitor callable, invocable with (at least) the
        /// active alternative's type.
        /// @param func Visitor callable.
        /// @return Wrapped visitor result, or a `runtime_error` if the
        /// variant is valueless (`index() == npos`).
        template <typename Func>
        constexpr auto visit(Func&& func)
        {
            return _storage.visit(_stored_index, hsd::forward<Func>(func));
        }

        /// @brief Const-qualified overload of visit(Func&&).
        /// @param func Visitor callable.
        /// @return Wrapped visitor result, or a `runtime_error` if the
        /// variant is valueless (`index() == npos`).
        template <typename Func>
        constexpr auto visit(Func&& func) const
        {
            return _storage.visit(_stored_index, hsd::forward<Func>(func));
        }

        /// @brief Get the index of the currently active alternative.
        /// @return Zero-based index into `Ts...`, or `npos` if the variant is valueless.
        constexpr usize index() const
        {
            return _stored_index;
        }

        /// @brief Compare two variants for equality.
        /// @details Requires equal active indices; then compares the active
        /// values via `operator==`.
        /// @param rhs Variant to compare against.
        /// @return `true` if both hold the same active index and equal values.
        constexpr bool operator==(const variant& rhs) const 
        {
            if (index() != rhs.index())
            {
                return false;
            }

            return _storage.visit(_stored_index, [&rhs]<typename T>(const T& val)
                {
                    return rhs.get<T>().unwrap().get() == val;
                }
            ).unwrap();
        }

        /// @brief Inequality complement of operator==().
        /// @param rhs Variant to compare against.
        /// @return `true` if the variants are not equal.
        constexpr bool operator!=(variant const& rhs) const
        {
            return !(*this == rhs);
        }
    };
}