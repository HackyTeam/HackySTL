#pragma once

#include "Utility.hpp"
#include "Allocator.hpp"
#include "Types.hpp"

namespace hsd
{
    namespace unique_detail
    {
        template <typename T, typename U>
        concept ConvertibleDerived = std::is_convertible_v<U, T> || std::is_base_of_v<T, U>;
        template <typename T>
        concept ArrayPointer = is_array<T>::value;
        template <typename T>
        concept ValuePointer = !ArrayPointer<T>;

        template < typename T, typename Allocator >
        class storage : private Allocator
        {
        private:
            usize _size = 0;
            using pointer_type = typename Allocator::pointer_type;
            using value_type = typename Allocator::value_type;

        public:
            HSD_CONSTEXPR storage()
            requires (std::is_default_constructible_v<Allocator>) = default;

            HSD_CONSTEXPR storage(pointer_type ptr, usize size)
            requires (std::is_default_constructible_v<Allocator>)
                : _size{size}
            {
                this->_data = ptr;
            }

            HSD_CONSTEXPR storage(const Allocator& alloc, usize size)
            requires (std::is_default_constructible_v<value_type>)
                : Allocator(alloc), _size{size}
            {
                this->_data = this->allocate(size);
                std::construct_at(this->_data);

                for(usize _index = 0; _index < size; _index++)
                {
                    std::construct_at(&this->_data[_index]);
                }
            }

            HSD_CONSTEXPR storage(pointer_type ptr, const Allocator& alloc, usize size)
                : Allocator(alloc), _size{size}
            {
                this->_data = ptr;
            }

            storage(const storage&) = delete;

            template < typename U, typename Alloc >
            HSD_CONSTEXPR storage(storage<U, Alloc>&& other)
                : Allocator(static_cast<Alloc&&>(other))
            {
                set_pointer(other.get_pointer());
                set_size(other.get_size());
                other.set_pointer(nullptr);
                other.set_size(0);
            }

            template < typename U, typename Alloc >
            HSD_CONSTEXPR storage& operator=(storage<U, Alloc>&& rhs)
            {
                set_pointer(rhs.get_pointer());
                set_size(rhs.get_size());
                rhs.set_pointer(nullptr);
                rhs.set_size(0);
                return *this;
            }

            constexpr Allocator& get_allocator()
            {
                return *this;
            }

            constexpr usize get_size() const
            {
                return _size;
            }

            constexpr auto* get_pointer() const
            {
                return this->_data;
            }

            constexpr void set_pointer(pointer_type ptr)
            {
                this->_data = ptr;
            }

            constexpr void set_size(usize size)
            {
                _size = size;
            }
        };

    } // namespace unique_detail

    template < typename T, typename Allocator = allocator<remove_array_t<T>> >
    class unique_ptr
    {
    private:
        unique_detail::storage<T, Allocator> _value;

        template <typename U, typename Alloc>
        friend class unique_ptr;

        HSD_CONSTEXPR void _delete()
        {
            _value.get_allocator().deallocate(
                _value.get_pointer(), _value.get_size());
            _value.set_pointer(nullptr);

        }

    public:
        using pointer_type = typename Allocator::pointer_type;
        using reference_type = typename Allocator::value_type&;
        
        HSD_CONSTEXPR unique_ptr() = default;
        HSD_CONSTEXPR unique_ptr(NullType) {}
        unique_ptr(unique_ptr&) = delete;
        unique_ptr(const unique_ptr&) = delete;

        HSD_CONSTEXPR unique_ptr(pointer_type ptr) 
        requires (std::is_default_constructible_v<Allocator>)
            : _value{ptr, 1u}
        {}

        HSD_CONSTEXPR unique_ptr(pointer_type ptr, usize size) 
        requires (std::is_default_constructible_v<Allocator>)
            : _value{ptr, size}
        {}

        HSD_CONSTEXPR unique_ptr(const Allocator& alloc) 
            : _value{alloc, 1u}
        {}

        HSD_CONSTEXPR unique_ptr(const Allocator& alloc, usize size) 
            : _value{alloc, size}
        {}

        HSD_CONSTEXPR unique_ptr(pointer_type ptr, const Allocator& alloc, usize size) 
            : _value{ptr, alloc, size}
        {}

        template <typename U = T> requires(unique_detail::ConvertibleDerived<T, U>)
        HSD_CONSTEXPR unique_ptr(unique_ptr<U>&& other)
            : _value{move(other._value)}
        {}

        HSD_CONSTEXPR ~unique_ptr()
        {
            _delete();
        }

        HSD_CONSTEXPR unique_ptr& operator=(NullType)
        {
            _delete();
            return *this;
        }

        template <typename U = T> requires(unique_detail::ConvertibleDerived<T, U>)
        HSD_CONSTEXPR unique_ptr& operator=(unique_ptr<U>&& rhs)
        {
            _value.operator=(move(rhs._value));
            rhs._delete();
            return *this;
        }

        constexpr auto* get()
        {
            return _value.get_pointer();
        }

        constexpr auto* get() const
        {
            return _value.get_pointer();
        }

        constexpr bool operator!=(NullType) const
        {
            return _value.get_pointer() != nullptr;
        }

        constexpr auto* operator->()
        {
            return get();
        }

        constexpr auto* operator->() const
        {
            return get();
        }

        constexpr auto& operator*()
        {
            return *get();
        }

        constexpr auto& operator*() const
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
    
    template <typename T, typename Allocator = allocator<remove_array_t<T>>, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T, Allocator>::single_object 
    make_unique(Args&&... args) requires (std::is_default_constructible_v<Allocator>)
    {
        Allocator _alloc;
        auto* _ptr = _alloc.allocate(1);
        std::construct_at(_ptr, forward<Args>(args)...);
        return unique_ptr<T, Allocator>(_ptr);
    }

    template <typename T, typename Allocator = allocator<remove_array_t<T>>, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T, Allocator>::single_object 
    make_unique(const Allocator& alloc, Args&&... args)
    {
        auto* _ptr = alloc.allocate(1);
        std::construct_at(_ptr, forward<Args>(args)...);
        return unique_ptr<T, Allocator>(alloc, 1);
    }

    template <typename T, typename Allocator = allocator<remove_array_t<T>>, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T, Allocator>::array
    make_unique(usize size) requires (std::is_default_constructible_v<Allocator>)
    {
        Allocator _alloc;
        auto* _ptr = _alloc.allocate(size);
        return unique_ptr<T, Allocator>(_ptr, size);
    }

    template <typename T, typename Allocator = allocator<remove_array_t<T>>, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T, Allocator>::array
    make_unique(const Allocator& alloc, usize size)
    {
        return unique_ptr<T, Allocator>(alloc, size);
    }

    template <typename T, typename Allocator = allocator<remove_array_t<T>>, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T, Allocator>::invalid_type 
    make_unique(Args&&...) requires (std::is_default_constructible_v<Allocator>) = delete;

    template <typename T, typename Allocator = allocator<remove_array_t<T>>, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T, Allocator>::invalid_type 
    make_unique(const Allocator&, Args&&...) = delete;
} // namespace hsd
