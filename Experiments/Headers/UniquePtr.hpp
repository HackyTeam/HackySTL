#pragma once

#include "Allocator.hpp"
#include "Concepts.hpp"

namespace hsd
{
    namespace unique_detail
    {
        template <typename T, typename U>
        concept ConvertibleDerived = Convertible<U, T> || std::is_base_of_v<T, U>;

        template < typename T, typename Allocator >
        class storage
        {
        public:
            using alloc_type = Allocator;
            using pointer_type = remove_array_t<T>*;
            using value_type = remove_array_t<T>;

        private:
            alloc_type _alloc;
            pointer_type _data = nullptr;
            usize _size = 0;
            
            template <typename U, typename Alloc>
            friend class storage;

        public:
            inline storage()
            requires (DefaultConstructible<alloc_type>) = default;

            inline storage(pointer_type ptr, usize size)
            requires (DefaultConstructible<alloc_type>)
                : _data{ptr}, _size{size}
            {}

            inline storage(const alloc_type& alloc, usize size)
            requires (DefaultConstructible<value_type>)
                : _alloc{alloc}, _size{size}
            {
                _data = _alloc
                    .template allocate<value_type>(size)
                    .unwrap();
                
                construct_at(_data);

                for(usize _index = 0; _index < size; _index++)
                {
                    construct_at(&_data[_index]);
                }
            }

            inline storage(pointer_type ptr, const alloc_type& alloc, usize size)
                : _alloc{alloc}, _data{ptr}, _size{size}
            {}

            inline storage(const storage&) = delete;

            template <typename U = T>
            inline storage(storage<U, Allocator>&& other)
            requires (MoveConstructible<alloc_type>)
                : _alloc{move(other._alloc)}, 
                _data{exchange(other._data, nullptr)},
                _size{exchange(other._size, 0)}
            {}

            template <typename U = T>
            inline storage(storage<U, Allocator>&& other)
            requires (!MoveConstructible<alloc_type>)
                : _data{exchange(other._data, nullptr)},
                _size{exchange(other._size, 0)}
            {}

            template <typename U = T>
            inline storage& operator=(storage<U, Allocator>&& rhs)
            {
                _alloc = move(rhs._alloc);
                _data = exchange(rhs._data, nullptr);
                _size = exchange(rhs._size, 0);
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

    template < typename T, typename Allocator = hsd_test::allocator >
    class unique_ptr
    {
    private:
        using storage_type = unique_detail::storage<T, Allocator>;
        storage_type _value;

        template <typename U, typename Alloc>
        friend class unique_ptr;

    public:
        using alloc_type = Allocator;
        using pointer_type = typename storage_type::pointer_type;
        using value_type = typename storage_type::value_type;
        using reference_type = typename storage_type::value_type&;

    private:
        inline void _delete()
        {
            if (get() != nullptr)
            {
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
            _delete();
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

        inline bool operator!=(NullType) const
        {
            return _value.get_pointer() != nullptr;
        }

        inline bool operator==(NullType) const
        {
            return _value.get_pointer() == nullptr;
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

    template < typename T, typename Allocator >
    struct MakeUniq
    {
        using single_object = unique_ptr<T, Allocator>;
    };

    template < typename T, typename Allocator >
    struct MakeUniq<T[], Allocator>
    {
        using array = unique_ptr<T[], Allocator>;
    };
    
    template < typename T, usize N, typename Allocator >
    struct MakeUniq<T[N], Allocator>
    {
        struct invalid_type {};  
    };
    
    template <typename T, typename Allocator = hsd_test::allocator, typename... Args>
    requires (DefaultConstructible<Allocator> && IsSame<remove_array_t<T>, T>)
    static inline typename MakeUniq<T, Allocator>::single_object make_unique(Args&&... args)
    {
        Allocator _alloc;
        auto _ptr = _alloc
            .template allocate<remove_array_t<T>>(1)
            .unwrap();
        
        construct_at(_ptr, hsd::forward<Args>(args)...);
        return unique_ptr<T, Allocator>{_ptr};
    }

    template <typename T, typename Allocator = hsd_test::allocator, typename... Args>
    static inline typename MakeUniq<T, Allocator>::single_object make_unique(Allocator& alloc, Args&&... args)
    requires (IsSame<remove_array_t<T>, T>)
    {
        auto* _ptr = alloc
            .template allocate<remove_array_t<T>>(1)
            .unwrap();
        
        construct_at(_ptr, hsd::forward<Args>(args)...);
        return unique_ptr<T, Allocator>{_ptr, alloc, 1};
    }

    template <typename T, typename Allocator = hsd_test::allocator>
    static inline typename MakeUniq<T, Allocator>::array make_unique(usize size)
    requires (DefaultConstructible<Allocator> && !IsSame<remove_array_t<T>, T>)
    {
        Allocator _alloc;
        auto* _ptr = _alloc
            .template allocate<remove_array_t<T>>(size)
            .unwrap();
        
        return unique_ptr<T, Allocator>{_ptr, size};
    }

    template <typename T, typename Allocator = hsd_test::allocator>
    static inline typename MakeUniq<T, Allocator>::array make_unique(Allocator& alloc, usize size)
    requires (!IsSame<remove_array_t<T>, T>)
    {
        return unique_ptr<T, Allocator>{alloc, size};
    }

    template <typename T, typename Allocator = hsd_test::allocator, typename... Args>
    static inline typename MakeUniq<T, Allocator>::invalid_type make_unique(const Allocator&, Args&&...) = delete;
} // namespace hsd
