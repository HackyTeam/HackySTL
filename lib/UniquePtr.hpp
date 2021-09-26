#pragma once

#include "Allocator.hpp"
#include "TypeTraits.hpp"
#include "Types.hpp"

namespace hsd
{
    namespace unique_detail
    {
        template <typename T, typename U>
        concept ConvertibleDerived = std::is_convertible_v<U, T> || std::is_base_of_v<T, U>;

        template < typename T, template <typename> typename Allocator >
        class storage
        {
        public:
            using alloc_type = Allocator<remove_array_t<T>>;
            using pointer_type = typename alloc_type::pointer_type;
            using value_type = typename alloc_type::value_type;

        private:
            alloc_type _alloc;
            pointer_type _data = nullptr;
            usize _size = 0;
            
            template <typename U, template <typename> typename Alloc>
            friend class storage;

        public:
            inline storage()
            requires (DefaultConstructible<alloc_type>) = default;

            inline storage(pointer_type ptr, usize size)
            requires (DefaultConstructible<alloc_type>)
                : _data{ptr}, _size{size}
            {}

            inline storage(const alloc_type& alloc, usize size)
            requires (CopyConstructible<alloc_type> && DefaultConstructible<value_type>)
                : _alloc{alloc}, _size{size}
            {
                _data = _alloc.allocate(size).unwrap();
                _alloc.construct_at(_data);

                for(usize _index = 0; _index < size; _index++)
                {
                    _alloc.construct_at(&_data[_index]);
                }
            }

            inline storage(pointer_type ptr, const alloc_type& alloc, usize size)
            requires (CopyConstructible<alloc_type>)
                : _alloc{alloc}, _data{ptr}, _size{size}
            {}

            inline storage(const storage&) = delete;

            template <typename U = T>
            inline storage(storage<U, Allocator>&& other)
            requires (MoveConstructible<alloc_type>)
                : _alloc(move(other._alloc))
            {
                _data = exchange(other._data, nullptr);
                swap(_size, other._size);
            }

            template <typename U = T>
            inline storage(storage<U, Allocator>&& other)
            requires (!MoveConstructible<alloc_type>)
            {
                _data = exchange(other._data, nullptr);
                swap(_size, other._size);
            }

            template <typename U = T>
            inline storage& operator=(storage<U, Allocator>&& rhs)
            {
                _alloc.deallocate(_data, _size).unwrap();
                _alloc = rhs._alloc;
                _data = exchange(rhs._data, nullptr);
                swap(_size, rhs._size);
                return *this;
            }

            inline auto deallocate()
            {
                return _alloc.deallocate(_data, _size);
            }

            inline usize get_size() const
            {
                return _size;
            }

            inline auto* get_pointer() const
            {
                return _data;
            }

            inline void set_pointer(pointer_type ptr)
            {
                _data = ptr;
            }

            inline void set_size(usize size)
            {
                _size = size;
            }
        };
    } // namespace unique_detail

    template < typename T, template <typename> typename Allocator = allocator >
    class unique_ptr
    {
    private:
        unique_detail::storage<T, Allocator> _value;

        template <typename U, template <typename> typename Alloc>
        friend class unique_ptr;

        inline void _delete()
        {
            if (get() != nullptr)
            {
                using value_type = typename unique_detail::storage<T, Allocator>::value_type;

                if constexpr (is_array<T>::value)
                {
                    for (usize i = 0, size = _value.get_size(); i < size; ++i)
                        get()[size - i].~value_type();
                }
                else
                {
                    get()->~value_type();
                }
            }
            
            _value.deallocate().unwrap();
            _value.set_pointer(nullptr);
        }

    public:
        using alloc_type = Allocator< remove_array_t<T> >;
        using pointer_type = typename alloc_type::pointer_type;
        using value_type = typename alloc_type::value_type;
        using reference_type = typename alloc_type::value_type&;
        
        inline unique_ptr() = default;
        inline unique_ptr(NullType) {}
        unique_ptr(unique_ptr&) = delete;
        unique_ptr(const unique_ptr&) = delete;

        inline unique_ptr(pointer_type ptr) 
            : _value{ptr, 1u}
        {}

        inline unique_ptr(pointer_type ptr, usize size) 
            : _value{ptr, size}
        {}

        inline unique_ptr(const alloc_type& alloc) 
            : _value{alloc, 1u}
        {}

        inline unique_ptr(const alloc_type& alloc, usize size) 
            : _value{alloc, size}
        {}

        inline unique_ptr(pointer_type ptr, 
            const alloc_type& alloc, usize size) 
            : _value{ptr, alloc, size}
        {}

        template <typename U = T> 
        requires(unique_detail::ConvertibleDerived<T, U>)
        inline unique_ptr(unique_ptr<U, Allocator>&& other)
            : _value{move(other._value)}
        {}

        inline ~unique_ptr()
        {
            _delete();
        }

        inline unique_ptr& operator=(NullType)
        {
            _delete();
            return *this;
        }

        template <typename U = T> 
        requires(unique_detail::ConvertibleDerived<T, U>)
        inline unique_ptr& operator=(unique_ptr<U, Allocator>&& rhs)
        {
            _value = move(rhs._value);
            return *this;
        }

        inline auto* get()
        {
            return _value.get_pointer();
        }

        inline auto* get() const
        {
            return _value.get_pointer();
        }

        inline bool operator==(NullType) const
        {
            return get() != nullptr;
        }

        inline bool operator==(const unique_ptr& o) const
        {
            return get() == o.get();
        }

        inline bool operator==(T* o) const
        {
            return get() == o;
        }

        template <typename To>
        requires (!is_same<To, unique_ptr>::value)
        friend inline bool operator==(const To& o, const unique_ptr& p)
        {
            return p == o;
        }

        template <typename To>
        friend inline bool operator!=(const unique_ptr& p, const To& o)
        {
            return !(p == o);
        }

        template <typename To>
        requires (!is_same<To, unique_ptr>::value)
        friend inline bool operator!=(const To& o, const unique_ptr& p)
        {
            return !(p == o);
        }

        explicit inline operator bool() const
        {
            return get() != nullptr;
        }

        inline auto* operator->()
        {
            return get();
        }

        inline auto* operator->() const
        {
            return get();
        }

        inline auto& operator*()
        {
            return *get();
        }

        inline auto& operator*() const
        {
            return *get();
        }
    };

    template < typename T, template <typename> typename Allocator >
    struct MakeUniq
    {
        using single_object = unique_ptr<T, Allocator>;
    };

    template < typename T, template <typename> typename Allocator >
    struct MakeUniq<T[], Allocator>
    {
        using array = unique_ptr<T[], Allocator>;
    };
    
    template < typename T, usize N, template <typename> typename Allocator >
    struct MakeUniq<T[N], Allocator>
    {
        struct invalid_type {};  
    };
    
    template <typename T, template <typename> typename Allocator = allocator, typename... Args>
    requires (DefaultConstructible<Allocator<uchar>> && is_same<
        typename Allocator<remove_array_t<T>>::pointer_type, remove_array_t<T>*>::value)
    static inline typename MakeUniq<T, Allocator>::single_object make_unique(Args&&... args)
    {
        Allocator<remove_array_t<T>> _alloc;
        auto _ptr = _alloc.allocate(1).unwrap();
        _alloc.construct_at(_ptr, forward<Args>(args)...);
        return unique_ptr<T, Allocator>(_ptr);
    }

    template <typename T, template <typename> typename Allocator = allocator, typename... Args>
    requires (DefaultConstructible<Allocator<uchar>> && !is_same<
        typename Allocator<remove_array_t<T>>::pointer_type, remove_array_t<T>*>::value)
    static inline typename MakeUniq<T, Allocator>::single_object make_unique(Args&&... args)
    {
        Allocator<remove_array_t<T>> _alloc;
        auto& _ptr = _alloc.allocate(1).unwrap();
        _alloc.construct_at(&_ptr[0], forward<Args>(args)...);
        return unique_ptr<T, Allocator>(_ptr);
    }

    template <typename T, template <typename> typename Allocator = allocator, typename U, typename... Args>
    static inline typename MakeUniq<T, Allocator>::single_object make_unique(Allocator<U>& alloc, Args&&... args)
    requires (!DefaultConstructible<Allocator<uchar>>)
    {
        auto* _ptr = static_cast<Allocator<remove_array_t<T>>>(alloc).allocate(1).unwrap();
        alloc.construct_at(_ptr, forward<Args>(args)...);
        return unique_ptr<T, Allocator>(static_cast<Allocator<remove_array_t<T>>>(alloc), 1);
    }

    template <typename T, template <typename> typename Allocator = allocator>
    static inline typename MakeUniq<T, Allocator>::array make_unique(usize size)
    requires (DefaultConstructible<Allocator<uchar>>)
    {
        Allocator<remove_array_t<T>> _alloc;
        auto* _ptr = _alloc.allocate(size).unwrap();
        return unique_ptr<T, Allocator>(_ptr, size);
    }

    template <typename T, template <typename> typename Allocator = allocator, typename U>
    static inline typename MakeUniq<T, Allocator>::array make_unique(Allocator<U>& alloc, usize size)
    {
        return unique_ptr<T, Allocator>(static_cast<Allocator<remove_array_t<T>>>(alloc), size);
    }

    template <typename T, typename U, template <typename> typename Allocator = allocator, typename... Args>
    static inline typename MakeUniq<T, Allocator>::invalid_type make_unique(const Allocator<U>&, Args&&...) = delete;
} // namespace hsd
