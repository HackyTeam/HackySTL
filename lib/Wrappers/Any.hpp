#pragma once

/// @file Any.hpp
/// @brief Copyable type-erased value storage with a small inline buffer.
/// @details Owns one erased value, supports checked typed access, pointer
/// access, type queries, reset, swap, and in-place construction. Small
/// nothrow-movable values live inside the `any` itself, larger ones in one
/// heap allocation, so holding an `int`, a pointer or a short string view
/// never touches the allocator.

#include "../Base/Allocator.hpp"
#include "Reference.hpp"

namespace hsd
{
    /// @brief Error returned when any is queried for the wrong type.
    struct bad_any_cast
    {
        /// @brief Gets a human-readable description of this error.
        /// @return A stable diagnostic string.
        constexpr const char* pretty_error() const
        {
            return "Illegal casting:\n"
            "Cannot cast into different type";
        }
    };

    namespace any_detail
    {
        /// @brief Bytes of inline storage inside every `any`.
        inline constexpr usize buffer_size = 3 * sizeof(void*);

        /// @brief Tells whether a value of type `T` is stored inside the `any`.
        /// @details The value must fit, be no more aligned than a pointer and
        ///          relocate without throwing, because moving an `any` moves
        ///          its inline value.
        template <typename T>
        inline constexpr bool stored_inline = (
            sizeof(T) <= buffer_size
            && alignof(T) <= alignof(void*)
            && hsd::is_nothrow_constructible<T, T&&>::value
        );

        /// @brief Inline bytes or the owning pointer of a heap-held value.
        union storage
        {
            /// @brief Heap allocation holding the value, for large types.
            void* heap;
            /// @brief Inline bytes holding the value, for small types.
            alignas(void*) unsigned char bytes[buffer_size];
        };

        /// @brief Type-specific operations behind one erased value.
        /// @details The address of the per-type table also identifies the
        ///          type, so typed access compares a single pointer.
        struct vtable
        {
            /// @brief Destroys the value and releases its heap block, if any.
            void (*destroy)(storage&) noexcept;
            /// @brief Copy-constructs the value into empty storage.
            void (*copy)(const storage& from, storage& to);
            /// @brief Relocates the value into empty storage and empties `from`.
            void (*move)(storage& from, storage& to) noexcept;
        };

        /// @brief Address of the value of type `T` held in `s`.
        template <typename T>
        inline T* address(storage& s) noexcept
        {
            if constexpr (stored_inline<T>)
            {
                return hsd::launder(reinterpret_cast<T*>(s.bytes));
            }
            else
            {
                return static_cast<T*>(s.heap);
            }
        }

        /// @brief Const overload of `address`.
        template <typename T>
        inline const T* address(const storage& s) noexcept
        {
            if constexpr (stored_inline<T>)
            {
                return hsd::launder(reinterpret_cast<const T*>(s.bytes));
            }
            else
            {
                return static_cast<const T*>(s.heap);
            }
        }

        /// @brief Constructs a `T` in empty storage from `args`.
        template <typename T, typename... Args>
        inline T* construct_in(storage& s, Args&&... args)
        {
            if constexpr (stored_inline<T>)
            {
                return hsd::construct_at(
                    reinterpret_cast<T*>(s.bytes), hsd::forward<Args>(args)...
                );
            }
            else
            {
                T* _block = allocator::allocate<T>(1).unwrap();
                s.heap = _block;
                return hsd::construct_at(_block, hsd::forward<Args>(args)...);
            }
        }

        /// @brief Destroys the `T` in `s` and releases its heap block.
        template <typename T>
        inline void destroy_in(storage& s) noexcept
        {
            T* _value = address<T>(s);
            _value->~T();

            if constexpr (!stored_inline<T>)
            {
                allocator::deallocate(_value, 1).unwrap();
            }
        }

        /// @brief Copies the `T` held in `from` into empty storage `to`.
        template <typename T>
        inline void copy_in(const storage& from, storage& to)
        {
            construct_in<T>(to, *address<T>(from));
        }

        /// @brief Moves the `T` held in `from` into empty storage `to`.
        /// @details Heap-held values keep their address; inline values are
        ///          move-constructed and the source is destroyed.
        template <typename T>
        inline void move_in(storage& from, storage& to) noexcept
        {
            if constexpr (stored_inline<T>)
            {
                T* _source = address<T>(from);
                hsd::construct_at(
                    reinterpret_cast<T*>(to.bytes), static_cast<T&&>(*_source)
                );
                _source->~T();
            }
            else
            {
                to.heap = from.heap;
            }
        }

        /// @brief The operation table, and identity, of `T`.
        template <typename T>
        inline constexpr vtable vtable_of = {
            &destroy_in<T>, &copy_in<T>, &move_in<T>
        };
    } // namespace any_detail

    /// @brief Copyable container for one value of any copy-constructible type.
    class any
    {
    private:
        /// @brief Inline bytes or heap pointer of the held value.
        any_detail::storage _store;
        /// @brief Operation table of the held type, or null when empty.
        const any_detail::vtable* _table = nullptr;

        /// @brief Gets the held value when it is a `U`.
        /// @return Pointer to the value, or null when empty or another type.
        template <typename U>
        constexpr U* _value_if()
        {
            if (_table != &any_detail::vtable_of<U>)
            {
                return nullptr;
            }

            return any_detail::address<U>(_store);
        }

        /// @brief Const overload of `_value_if`.
        /// @return Pointer to the value, or null when empty or another type.
        template <typename U>
        constexpr const U* _value_if() const
        {
            if (_table != &any_detail::vtable_of<U>)
            {
                return nullptr;
            }

            return any_detail::address<U>(_store);
        }

        /// @brief Takes over the value of `other`, which must leave empty.
        /// @details This must be empty.
        constexpr void _take(any& other) noexcept
        {
            if (other._table != nullptr)
            {
                other._table->move(other._store, _store);
                _table = hsd::exchange(other._table, nullptr);
            }
        }

    public:
        /// @brief Constructs an empty any.
        constexpr any() noexcept = default;

        /// @brief Stores a copy of `other`.
        /// @tparam T Copy-constructible value type.
        /// @param other Value moved into the any.
        template <typename T>
        requires (!IsSame<T, any> && CopyConstructible<T>)
        constexpr any(T other)
        {
            any_detail::construct_in<T>(_store, hsd::move(other));
            _table = &any_detail::vtable_of<T>;
        }

        /// @brief Copies the value held by `other`, if any.
        /// @param other Source any.
        constexpr any(const any& other)
        {
            if (other._table != nullptr)
            {
                other._table->copy(other._store, _store);
                _table = other._table;
            }
        }

        /// @brief Takes the value of `other`, leaving it empty.
        /// @param other Source any.
        constexpr any(any&& other) noexcept
        {
            _take(other);
        }

        /// @brief Destroys the held value.
        constexpr ~any()
        {
            reset();
        }

        /// @brief Replaces the held value with a copy of, or the value of, `rhs`.
        /// @param rhs Source passed by value, so it is already copied or moved.
        /// @return `*this`.
        constexpr any& operator=(any rhs) noexcept
        {
            this->swap(rhs);
            return *this;
        }

        /// @brief Returns a checked reference to the held `T`.
        /// @tparam T Requested type.
        /// @return A reference, or `bad_any_cast` when another type is held.
        template <typename T>
        constexpr auto cast_to()
            -> result<reference<T>, bad_any_cast>
        {
            using type = typename remove_pointer<T>::type;

            if (auto* p_value = _value_if<type>())
            {
                return reference<T>{*p_value};
            }
            else
            {
                return bad_any_cast{};
            }
        }

        /// @brief Returns a checked const reference to the held `T`.
        /// @tparam T Requested type.
        /// @return A const reference, or `bad_any_cast` when another type is held.
        template <typename T>
        constexpr auto cast_to() const
            -> result<reference<const T>, bad_any_cast>
        {
            using type = typename remove_pointer<T>::type;

            if (auto* p_value = _value_if<type>())
            {
                return reference<const T>{*p_value};
            }
            else
            {
                return bad_any_cast{};
            }
        }

        /// @brief Returns a pointer to the held `T`.
        /// @tparam T Requested type.
        /// @return The pointer, or null when another type or nothing is held.
        template <typename T>
        constexpr T* cast_if()
        {
            return _value_if<T>();
        }

        /// @brief Returns a pointer to the held `T` through a const `any`.
        /// @tparam T Requested type.
        /// @return A const pointer, or null when another type or nothing is held.
        template <typename T>
        constexpr const T* cast_if() const
        {
            return _value_if<T>();
        }

        /// @brief Tells whether the held value is exactly a `T`.
        /// @tparam T Type to test.
        /// @return True when a `T` is held.
        template <typename T>
        constexpr bool holds_type() const
        {
            return _value_if<T>() != nullptr;
        }

        /// @brief Exchanges the held values.
        /// @param other Any to exchange with.
        constexpr void swap(any& other) noexcept
        {
            if (this == &other)
            {
                return;
            }

            any _tmp{};
            _tmp._take(other);
            other._take(*this);
            this->_take(_tmp);
        }

        /// @brief Exchanges two any objects.
        friend void swap(any& lhs, any& rhs) noexcept
        {
            lhs.swap(rhs);
        }

        /// @brief Replaces the held value with a `T` built from `args`.
        /// @details The new value is built before the old one is released, so
        ///          `args` may refer to the old value.
        /// @tparam T Type to construct.
        /// @param args Constructor arguments.
        template <typename T, typename... Args>
        constexpr void emplace(Args&&... args)
        {
            any _fresh{};
            any_detail::construct_in<T>(_fresh._store, T(hsd::forward<Args>(args)...));
            _fresh._table = &any_detail::vtable_of<T>;
            this->swap(_fresh);
        }

        /// @brief Tells whether a value is held.
        /// @return True unless empty.
        constexpr bool has_value() const
        {
            return _table != nullptr;
        }

        /// @brief Destroys the held value, if any.
        constexpr void reset()
        {
            if (_table != nullptr)
            {
                _table->destroy(_store);
                _table = nullptr;
            }
        }
    };
}
