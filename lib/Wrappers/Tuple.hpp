#pragma once

/// @file Tuple.hpp
/// @brief Compile-time heterogeneous tuple storage and composition.
/// @details Supports forwarding construction, indexed/type access, references,
/// tuple concatenation, and constexpr expansion through integer sequences.

#include "Reference.hpp"
#include "../Base/Concepts.hpp"
#include "Pair.hpp"
#include "../Base/IntegerSequence.hpp"

    /// @brief Matches types a tuple element can be built from with `T(args...)`.
    /// @details Brace initialization rejects reference element types, so the
    /// parenthesized form is used for them.
    template <typename T, typename... Args>
    concept ElementConstructible = requires(Args... args)
    {
        T(hsd::forward<Args>(args)...);
    };

namespace hsd
{
    /// @brief Heterogeneous tuple with compile-time indexed access.
    template <typename... Args>
    class tuple;

    template <typename... Args>
    static constexpr auto make_tuple(Args&&... args)
        -> tuple<decay_t<Args>...>;

    /// @brief Type equality helper for two tuple types.
    template < typename Tuple1, typename Tuple2 >
    using is_same_tuple = is_same<Tuple1, Tuple2>;

    template <typename>
    struct is_tuple : false_type {};

    template <typename... Args>
    struct is_tuple<tuple<Args...>> : true_type {};

    template <typename T>
    concept IsTuple = is_tuple<T>::value;

    /// @brief Empty tuple base case.
    template <>
    class tuple<>
    {
    public:
        template <typename... U>
        friend class tuple;

        /// @brief Returns zero because this tuple has no elements.
        /// @return Zero.
        static consteval usize size()
        {
            return 0;
        }

        /// @brief Returns the right-hand tuple because the left tuple is empty.
        /// @param rhs Tuple appended to this empty tuple.
        /// @return `rhs`.
        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) const
        {
            return rhs;
        }

        /// @brief Appends one non-tuple value to an empty tuple.
        /// @param rhs Value placed in the resulting one-element tuple.
        /// @return A one-element tuple containing `rhs`.
        template <typename U> requires (!is_tuple<U>::value)
        constexpr auto operator+(const U& rhs) const
        {
            return make_tuple(hsd::forward<U>(rhs));
        }
    };

    /// @brief Single-element tuple specialization.
    /// @tparam T Stored element type.
    template <typename T> requires (!IsReference<T>)
    class tuple<T>
    {   
    private:   
        /// @brief Stored tuple element.
        T _first;       

        template <typename... U>
        friend class tuple;

        /// @brief Returns the sole stored element without reference adaptation.
        /// @return Const reference to the stored element.
        template <usize N> requires (N == 0)
        constexpr const auto& get_raw() const
        {
            return _first;
        }
    public:
        /// @brief Value-initializes the sole element.
        constexpr tuple() = default;

        /// @brief Constructs the element from a compatible value.
        /// @param first Value forwarded to the element constructor.
        template <typename U> requires (Constructible<T, U>)
        constexpr tuple(U&& first) 
            : _first{hsd::forward<U>(first)}
        {}

        /// @brief Copies the sole element.
        /// @param other Tuple whose element is copied.
        constexpr tuple(const tuple& other)
            : _first{other._first}
        {}

        /// @brief Moves the sole element.
        /// @param other Tuple whose element is moved.
        constexpr tuple(tuple&& other)
            : _first{hsd::move(other._first)}
        {}

        /// @brief Copies the sole element into this tuple.
        /// @param other Tuple whose element is copied.
        /// @return `*this` after assignment.
        constexpr tuple& operator=(const tuple& other)
        {
            _first = other._first;
            return *this;
        }

        /// @brief Moves the sole element into this tuple.
        /// @param other Tuple whose element is moved.
        /// @return `*this` after assignment.
        constexpr tuple& operator=(tuple&& other)
        {
            _first = hsd::move(other._first);
            return *this;
        }

        /// @brief Concatenates this element with another tuple.
        /// @param rhs Tuple appended after the sole element.
        /// @return New tuple containing this element followed by `rhs`.
        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) const
        {
            return [&]<usize... Ints>(index_sequence<Ints...>)
            {
                return []<typename... TupArgs>(TupArgs... args)
                {
                    return make_tuple<TupArgs...>(hsd::move(args)...);
                }(get_raw<0>(), rhs.template get_raw<Ints>()...);
            }(index_sequence_for<Args...>{});
        }

        /// @brief Appends one non-tuple value after the sole element.
        /// @param rhs Value appended after the sole element.
        /// @return New tuple containing both values.
        template <typename U> requires (!is_tuple<U>::value)
        constexpr auto operator+(const U& rhs) const
        {
            return []<typename... TupArgs>(TupArgs... args)
            {
                return make_tuple<TupArgs...>(hsd::move(args)...);
            }(get_raw<0>(), rhs);
        }

        /// @brief Returns the sole element for index zero.
        /// @return Reference to the stored element.
        template <usize N> requires (N == 0)
        constexpr auto& get()
        {
            if constexpr (requires {{_first.get()} -> IsReference;})
            {
                return _first.get();
            }
            else
            {
                return _first;
            }
        }

        /// @brief Returns the sole element for const index-zero access.
        /// @return Const reference to the stored element.
        template <usize N> requires (N == 0)
        constexpr const auto& get() const
        {
            if constexpr (requires {{_first.get()} -> IsReference;})
            {
                return _first.get();
            }
            else
            {
                return _first;
            }
        }

        /// @brief Returns one because this tuple has one element.
        /// @return The compile-time tuple extent, equal to one.
        static consteval usize size()
        {
            return 1;
        }
    };

    template <typename T> requires (IsReference<T>)
    class tuple<T>
        : public tuple<reference<remove_reference_t<T>>>
    {};

    /// @brief Recursive tuple specialization storing a head and tail tuple.
    /// @tparam T First element type.
    /// @tparam Rest Remaining element types.
    template < typename T, typename... Rest > requires (!IsReference<T>)
    class tuple<T, Rest...>
    {   
    private:   
        /// @brief First stored tuple element.
        T _first;
        /// @brief Remaining elements stored recursively.
        tuple<Rest...> _rest;  

        template <typename... U>
        friend class tuple;      

        /// @brief Returns an element without reference adaptation.
        /// @tparam N Zero-based tuple index.
        /// @return Const reference to the selected element.
        template <usize N>
        constexpr const auto& get_raw() const
        {
            if constexpr (N == 0)
            {
                return _first;
            }
            else
            {
                return _rest.template get_raw<N - 1>();
            }
        }
    public:
        /// @brief Value-initializes all tuple elements.
        constexpr tuple() = default;

        /// @brief Constructs all elements from forwarded values.
        /// @param first Value forwarded to the first element.
        /// @param rest Values forwarded to the remaining elements.
        template <typename U, typename... Args> 
        requires (ElementConstructible<T, U> && ElementConstructible<tuple<Rest...>, Args...>)
        constexpr tuple(U&& first, Args&&... rest) 
            : _first{hsd::forward<U>(first)}, _rest{hsd::forward<Args>(rest)...}
        {}

        /// @brief Constructs a two-element tuple from a copied pair.
        /// @param value Pair whose elements initialize the tuple.
        template <typename U, typename V> 
        requires (Constructible<T, U> && Constructible<tuple<Rest...>, V>)
        constexpr tuple(const pair<U, V>& value) 
            : _first{value.first}, _rest{value.second}
        {}

        /// @brief Constructs a two-element tuple from a moved pair.
        /// @param value Pair whose elements are moved into the tuple.
        template <typename U, typename V> 
        requires (Constructible<T, U> && Constructible<tuple<Rest...>, V>)
        constexpr tuple(pair<U, V>&& value) 
            : _first{hsd::move(value.first)}, _rest{hsd::move(value.second)}
        {}

        /// @brief Copies all tuple elements.
        /// @param other Tuple whose elements are copied.
        constexpr tuple(const tuple& other)
            : _first{other._first}, _rest{other._rest}
        {}

        /// @brief Moves all tuple elements.
        /// @param other Tuple whose elements are moved.
        constexpr tuple(tuple&& other)
            : _first{hsd::move(other._first)}, _rest{hsd::move(other._rest)}
        {}

        /// @brief Copies all tuple elements into this tuple.
        /// @param other Tuple whose elements are copied.
        /// @return `*this` after assignment.
        constexpr tuple& operator=(const tuple& other)
        {
            _first = other._first;
            _rest = other._rest;
            return *this;
        }

        /// @brief Moves all tuple elements into this tuple.
        /// @param other Tuple whose elements are moved.
        /// @return `*this` after assignment.
        constexpr tuple& operator=(tuple&& other)
        {
            _first = hsd::move(other._first);
            _rest = hsd::move(other._rest);
            return *this;
        }

        /// @brief Concatenates this tuple with another tuple.
        /// @param rhs Tuple appended after this tuple.
        /// @return New tuple containing both sequences.
        template <typename... Args>
        constexpr auto operator+(const tuple<Args...>& rhs) const
        {
            return [&]< usize... Ints1, usize... Ints2 >
                (index_sequence<Ints1...>, index_sequence<Ints2...>)
            {
                return []<typename... TupArgs>(TupArgs... args)
                {
                    return make_tuple<TupArgs...>(hsd::move(args)...);
                }(get_raw<Ints1>()..., rhs.template get_raw<Ints2>()...);
            }(index_sequence_for<T, Rest...>{}, index_sequence_for<Args...>{});
        }

        /// @brief Appends one non-tuple value after this tuple.
        /// @param rhs Value appended after this tuple.
        /// @return New tuple containing the existing elements and `rhs`.
        template <typename U> requires (!is_tuple<U>::value)
        constexpr auto operator+(const U& rhs) const
        {
            return [&]< usize... Ints1>
                (index_sequence<Ints1...>)
            {
                return []<typename... TupArgs>(TupArgs... args)
                {
                    return make_tuple<TupArgs...>(hsd::move(args)...);
                }(get_raw<Ints1>()..., rhs);
            }(index_sequence_for<T, Rest...>{});
        }

        /// @brief Returns the element at compile-time index `N`.
        /// @tparam N Zero-based tuple index.
        /// @return Reference to the selected element.
        template <usize N>
        constexpr auto& get()
        {
            if constexpr (N == 0)
            {
                if constexpr (requires {{_first.get()} -> IsReference;})
                {
                    return _first.get();
                }
                else
                {
                    return _first;
                }
            }
            else
            {
                return _rest.template get<N - 1>();
            }
        }

        /// @brief Returns the element at compile-time index `N` for const access.
        /// @tparam N Zero-based tuple index.
        /// @return Const reference to the selected element.
        template <usize N>
        constexpr const auto& get() const
        {
            if constexpr(N == 0)
            {
                if constexpr (requires {{_first.get()} -> IsReference;})
                {
                    return _first.get();
                }
                else
                {
                    return _first;
                }
            }
            else
            {
                return _rest.template get<N - 1>();
            }
        }

        /// @brief Returns the number of stored elements.
        /// @return Number of stored tuple elements.
        static consteval usize size()
        {
            return 1 + sizeof...(Rest);
        }
    };

    /// @brief Multi-element tuple whose first element is a reference.
    /// @details Stores the first element as `reference<T>` through the base
    /// class, and inherits the base constructors so tuples can be built from values.
    template <typename T, typename... Rest> requires (IsReference<T>)
    class tuple<T, Rest...>
        : public tuple<reference<remove_reference_t<T>>, Rest...>
    {
    public:
        using tuple<reference<remove_reference_t<T>>, Rest...>::tuple;
    };

    /// @brief Recursive type lookup for a tuple of types.
    template <hsd::usize, typename...>
    struct type_tuple_impl;

    /// @brief Base case selecting the first type at index zero.
    template <typename T, typename... Ts>
    struct type_tuple_impl<0, T, Ts...>
    {
        /// @brief Type selected at the requested index.
        using type = T;
    };

    /// @brief Recursive type lookup after removing one leading type.
    template <hsd::usize Idx, typename T, typename... Ts>
    struct type_tuple_impl<Idx, T, Ts...>
    {
        /// @brief Remaining lookup after decrementing the index.
        using next = type_tuple_impl<Idx - 1, Ts...>;
        /// @brief Type selected at the requested index.
        using type = typename next::type;
    };

    /// @brief Compile-time size and indexed type metadata for a type pack.
    template <typename... Ts>
    struct type_tuple
    {
        /// @brief Number of types in the pack.
        static constexpr usize size = sizeof...(Ts);

        template <hsd::usize Idx>
        /// @brief Type at zero-based compile-time index `Idx`.
        using type_at = typename type_tuple_impl<Idx, Ts...>::type;
    };

    /// @brief Type metadata specialization for an hsd tuple.
    template <typename... Ts>
    struct type_tuple<tuple<Ts...>>
        : type_tuple<Ts...>
    {};

    /// @brief Deduction guide for tuple element values.
    template < typename... UTypes > tuple(UTypes...) -> tuple<UTypes...>;
    /// @brief Deduction guide for a pair converted to a two-element tuple.
    template < typename T1, typename T2 > tuple(pair<T1, T2>) -> tuple<T1, T2>;

    /// @brief Constructs a tuple with decayed element types.
    /// @param args Values forwarded into the tuple.
    /// @return Tuple containing decayed copies or moves of `args`.
    template <typename... Args>
    static constexpr auto make_tuple(Args&&... args)
        -> tuple<decay_t<Args>...>
    {
        return tuple<decay_t<Args>...>{hsd::forward<Args>(args)...};
    }

    /// @brief Constructs a tuple of references to existing objects.
    /// @param args Objects referenced by the resulting tuple.
    /// @return Tuple containing non-owning references to `args`.
    template <typename... Args>
    static constexpr auto tie(Args&... args)
        -> tuple<decay_t<Args>&...>
    {
        return tuple<decay_t<Args>&...>{args...};
    }

    /// @brief Expands tuple arguments into a member-function call for a pointer/value target.
    /// @param func Member function pointer.
    /// @param value Target object or pointer.
    /// @param args Tuple supplying call arguments.
    /// @return Result of the member-function invocation.
    template < typename Func, typename T, typename... Args, usize... Is >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply_impl(
        Func&& func, T&& value, index_sequence<Is...>, const tuple<Args...>& args) 
    {
        if constexpr (is_pointer<T>::value)
        {
            return (value->*func)(args.template get<Is>()...);
        }
        else
        {
            return (value.*func)(args.template get<Is>()...);
        }
    }
    
    /// @brief Applies a member function to a tuple of arguments.
    /// @param func Member function pointer.
    /// @param value Target object or pointer.
    /// @param args Tuple supplying call arguments.
    /// @return Result of the member-function invocation.
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply(Func&& func, T&& value, const tuple<Args...>& args)
    {
        return apply_impl(
            hsd::forward<Func>(func), hsd::forward<T>(value), 
            make_index_sequence<sizeof...(Args)>{}, args
        );
    }

    /// @brief Expands tuple arguments into a member-function call for a reference target.
    /// @param func Member function pointer.
    /// @param value Target object or pointer reference.
    /// @param args Tuple supplying call arguments.
    /// @return Result of the member-function invocation.
    template < typename Func, typename T, typename... Args, usize... Is >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply_impl(
        Func&& func, T& value, index_sequence<Is...>, const tuple<Args...>& args) 
    {
        if constexpr (is_pointer<T>::value)
        {
            return (value->*func)(args.template get<Is>()...);
        }
        else
        {
            return (value.*func)(args.template get<Is>()...);
        }
    }
    
    /// @brief Applies a member function to a tuple using a target reference.
    /// @param func Member function pointer.
    /// @param value Target object or pointer reference.
    /// @param args Tuple supplying call arguments.
    /// @return Result of the member-function invocation.
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static constexpr auto apply(Func&& func, T& value, const tuple<Args...>& args)
    {
        return apply_impl(
            hsd::forward<Func>(func), value, make_index_sequence<sizeof...(Args)>{}, args
        );
    }

    /// @brief Expands tuple arguments into a free-callable invocation.
    /// @param func Callable to invoke.
    /// @param args Tuple supplying call arguments.
    /// @return Result of the callable invocation.
    template < typename Func, typename... Args, usize... Is >
    requires (!std::is_member_function_pointer_v<Func>)
    static constexpr auto apply_impl(
        Func&& func, index_sequence<Is...>, const tuple<Args...>& args) 
    {
        return func(args.template get<Is>()...);
    }
    
    /// @brief Applies a free callable to tuple elements.
    /// @param func Callable to invoke.
    /// @param args Tuple supplying call arguments.
    /// @return Result of the callable invocation.
    template < typename Func, typename... Args >
    requires (!std::is_member_function_pointer_v<Func>)
    static constexpr auto apply(Func&& func, const tuple<Args...>& args)
    {
        return apply_impl(
            hsd::forward<Func>(func), make_index_sequence<sizeof...(Args)>{}, args
        );
    }
}