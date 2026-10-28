#pragma once

#include <Base/Result.hpp>
#include <Wrappers/Reference.hpp>

namespace hsd
{
    namespace variant_detail
    {
        template <usize Idx, typename T>
        struct internal_storage
        {
            static constexpr usize index = Idx;
            T value;

            constexpr ~internal_storage() {}
        };

        template <typename Func, typename Arg>
        using invoke_return_type = decltype(declval<Func>()(declval<Arg>()));

        template <typename Func, typename Arg>
        struct visit_return_type
        {
            using invoke_ret = invoke_return_type<Func, Arg>;

            using type = conditional_t<
                IsReference<invoke_ret>,
                reference<remove_reference_t<invoke_ret>>,
                invoke_ret
            >;
        };

        class error_placeholder;

        template <usize Idx, typename... T>
        class storage;

        // last index class
        template <usize Idx, typename T>
        class storage<Idx, T>
        {
        private:
            union {
                internal_storage<Idx, T> _first;
            };

        public:
            template <usize TypeIdx>
            using type_at = hsd::conditional_t<Idx == TypeIdx, T, error_placeholder>;

            template <typename U>
            using constructible_at = hsd::conditional_t<Constructible<T, U>, T, error_placeholder>;

            constexpr storage() {}
            constexpr storage(const storage&) = delete;
            constexpr storage(storage&&) = delete;

            constexpr ~storage() {}

            constexpr storage& operator=(const storage&) = delete;
            constexpr storage& operator=(storage&&) = delete;

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

            constexpr option_err<runtime_error> destroy(usize idx)
            {
                if (idx != Idx) [[unlikely]]
                {
                    return runtime_error{"Index is out of range"};
                }

                _first.value.~T();
                return {};
            }

            template <typename U, typename... Args>
            requires (Constructible<T, Args...> && hsd::IsSame<T, U>)
            constexpr void construct(Args&&... args)
            {
                construct_at(addressof(_first.value), hsd::forward<Args>(args)...);
            }

            template <typename U>
            requires (hsd::IsSame<T, U>)
            constexpr auto& get_value()
            {
                return _first.value;
            }

            template <typename U>
            requires (hsd::IsSame<T, U>)
            constexpr const auto& get_value() const
            {
                return _first.value;
            }

            template <usize ValIdx>
            requires (Idx == ValIdx)
            constexpr auto& get_value()
            {
                return _first.value;
            }

            template <usize ValIdx>
            requires (Idx == ValIdx)
            constexpr const auto& get_value() const
            {
                return _first.value;
            }

            template <typename U>
            requires (hsd::IsSame<T, U>)
            static consteval usize get_index()
            {
                return Idx;
            }
        };

        template <usize Idx, typename T, typename... Rest>
        class storage<Idx, T, Rest...>
        {
        private:
            using RestType = storage<Idx + 1, Rest...>;

            union {
                internal_storage<Idx, T> _first;
                RestType _rest;
            };

        public:
            template <usize TypeIdx>
            using type_at = conditional_t<Idx == TypeIdx, T, typename RestType::template type_at<TypeIdx>>;

            template <typename U>
            using constructible_at = conditional_t<
                Constructible<T, U>, T, typename RestType::template constructible_at<U>
            >;

            constexpr storage() {}
            constexpr storage(const storage&) = delete;
            constexpr storage(storage&&) = delete;

            constexpr ~storage() {}

            constexpr storage& operator=(const storage&) = delete;
            constexpr storage& operator=(storage&&) = delete;

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

            constexpr option_err<runtime_error> destroy(usize idx)
            {
                if (idx != Idx)
                {
                    return _rest.destroy(idx);
                }

                _first.value.~T();
                return {};
            }

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
                    construct_at(addressof(_first.value), hsd::forward<Args>(args)...);
                }
            }

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

    template <typename T>
    struct in_place_t {};

    struct monostate {};

    constexpr bool operator==(monostate, monostate) noexcept
    {
        return true;
    }

    template <typename... Ts>
    class variant
    {
    public:
        static constexpr usize npos = -1;

    private:
        using storage_type = conditional_t<
            (DefaultConstructible<Ts> || ...),
            variant_detail::storage<0, Ts...>,
            variant_detail::storage<npos, monostate, Ts...>
        >;

        usize _stored_index = npos;
        storage_type _storage = {};

        constexpr void _construct_from_variant(const variant& val)
        {
            val._storage.visit(
                _stored_index,
                [this]<typename T>(const T& val) {
                    _storage.template construct<T>(val);
                }
            ).unwrap();
        }

        constexpr void _construct_from_variant(variant&& val)
        {
            val._storage.visit(
                _stored_index,
                [this]<typename T>(T&& val) {
                    _storage.template construct<T>(move(val));
                }
            ).unwrap();
        }

        constexpr void _attribute_from_variant(const variant& val)
        {
            val._storage.visit(
                _stored_index,
                [this]<typename T>(const T& val) {
                    _storage.template get_value<T>() = val;
                }
            ).unwrap();
        }

        constexpr void _attribute_from_variant(variant&& val)
        {
            val._storage.visit(
                _stored_index,
                [this]<typename T>(T&& val) {
                    _storage.template get_value<T>() = move(val);
                }
            ).unwrap();
        }

        constexpr void _destroy_variant()
        {
            if (_stored_index != npos)
            {
                _storage.destroy(_stored_index).unwrap();
            }
        }

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
                _stored_index = storage_type::template get_index<T>();
                _storage.template construct<T>(hsd::forward<U>(rhs));
            }
        }

    public:
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

        constexpr ~variant()
        {
            _destroy_variant();
        }

        template <typename T>
        requires (!IsSame<remove_cvref_t<T>, variant<Ts...>>)
        constexpr variant(T&& val)
        {
            using DesiredType = typename storage_type::template constructible_at<T>;

            if constexpr (IsSame<DesiredType, remove_cvref_t<T>>)
            {
                _stored_index = storage_type::template get_index<T>();
                _storage.template construct<T>(hsd::forward<T>(val));
            }
            else
            {
                _stored_index = storage_type::template get_index<DesiredType>();
                _storage.template construct<DesiredType>(hsd::forward<T>(val));
            }
        }

        template <typename T, typename... Args>
        constexpr variant(in_place_t<T>, Args&&... args)
            : _stored_index{storage_type::template get_index<T>()}
        {
            _storage.template construct<T>(hsd::forward<Args>(args)...);
        }

        constexpr variant(const variant& val)
            : _stored_index{val._stored_index}
        {
            _construct_from_variant(val);
        }

        constexpr variant(variant&& val)
            : _stored_index{val._stored_index}
        {
            _construct_from_variant(move(val));
        }

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

        constexpr variant& operator=(variant&& rhs)
        {
            if (_stored_index == rhs._stored_index)
            {
                _attribute_from_variant(move(rhs));
            }
            else
            {
                _destroy_variant();
                _stored_index = rhs._stored_index;
                _construct_from_variant(move(rhs));
            }

            return *this;
        }

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

        template <typename T>
        constexpr T* get_if()
        {
            if (_stored_index != storage_type::template get_index<T>())
            {
                return nullptr;
            }

            return addressof(_storage.template get_value<T>());
        }

        template <typename T>
        constexpr const T* get_if() const
        {
            if (_stored_index != storage_type::template get_index<T>())
            {
                return nullptr;
            }

            return addressof(_storage.template get_value<T>());
        }

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

        template <usize Idx>
        constexpr auto get_if()
            -> typename storage_type::template type_at<Idx>*
        {
            if (_stored_index != Idx)
            {
                return nullptr;
            }

            return addressof(_storage.template get_value<Idx>());
        }

        template <usize Idx>
        constexpr auto get_if() const
            -> const typename storage_type::template type_at<Idx>*
        {
            if (_stored_index != Idx)
            {
                return nullptr;
            }

            return addressof(_storage.template get_value<Idx>());
        }

        template <typename T>
        constexpr bool holds() const
        {
            return _stored_index == storage_type::template get_index<T>();
        }

        template <typename Func>
        constexpr auto visit(Func&& func)
        {
            return _storage.visit(_stored_index, hsd::forward<Func>(func));
        }

        template <typename Func>
        constexpr auto visit(Func&& func) const
        {
            return _storage.visit(_stored_index, hsd::forward<Func>(func));
        }

        constexpr usize index() const
        {
            return _stored_index;
        }

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

        constexpr bool operator!=(variant const& rhs) const
        {
            return !(*this == rhs);
        }
    };
}