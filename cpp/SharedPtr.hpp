#pragma once

#include "Utility.hpp"
#include "_XUtility.hpp"
#include "Allocator.hpp"
#include "Atomic.hpp"

namespace hsd
{
    namespace non_atomic_types
    {
        namespace shared_detail
        {
            template <typename T, typename U>
            concept ConvertibleDerived = std::is_convertible_v<U, T> || std::is_base_of_v<T, U>;

            template < typename T, template <typename> typename Allocator >
            class storage
            {
            private:
                using alloc_type = Allocator<remove_array_t<T>>;
                using pointer_type = typename alloc_type::pointer_type;
                using value_type = typename alloc_type::value_type;

                alloc_type _alloc;
                pointer_type _data = nullptr;
                usize _size = 0;

                template <typename U, template <typename> typename Alloc>
                friend class storage;

            public:
                inline storage()
                requires (std::is_default_constructible_v<alloc_type>) = default;

                inline storage(pointer_type ptr, usize size)
                requires (std::is_default_constructible_v<alloc_type>)
                    : _data{ptr}, _size{size}
                {}

                inline storage(const alloc_type& alloc, usize size)
                requires (std::is_copy_constructible_v<value_type>)
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
                requires (std::is_copy_constructible_v<value_type>)
                    : _alloc{alloc}, _data{ptr}, _size{size}
                {}

                inline storage(const storage& other)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                inline storage(const storage& other)
                requires (!std::is_copy_constructible_v<alloc_type>)
                    : _data{other._data}, _size{other._size}
                {}

                inline storage(storage&& other)
                requires (std::is_move_constructible_v<alloc_type>)
                    : _alloc{move(other._alloc)}
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                inline storage(storage&& other)
                requires (!std::is_move_constructible_v<alloc_type>)
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                template <typename U = T>
                inline storage(const storage<U, Allocator>& other)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                template <typename U = T>
                inline storage(const storage<U, Allocator>& other)
                requires (!std::is_copy_constructible_v<alloc_type>)
                    : _data{other._data}, _size{other._size}
                {}

                template <typename U = T>
                inline storage(storage<U, Allocator>&& other)
                requires (std::is_move_constructible_v<alloc_type>)
                    : _alloc{move(other._alloc)}
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                template <typename U = T>
                inline storage(storage<U, Allocator>&& other)
                requires (!std::is_move_constructible_v<alloc_type>)
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                inline storage& operator=(const storage& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                inline storage& operator=(storage&& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = exchange(rhs._data, nullptr);                    
                    swap(_size, rhs._size);
                    return *this;
                }

                template <typename U = T>
                inline storage& operator=(const storage<U, Allocator>& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                template <typename U = T>
                inline storage& operator=(storage<U, Allocator>&& rhs)
                {
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
                    this->_data = ptr;
                }

                inline void set_size(usize size)
                {
                    _size = size;
                }
            };

            template < template <typename> typename Allocator >
            class counter
            {
            private:
                using alloc_type = Allocator<usize>;
                using pointer_type = typename alloc_type::pointer_type;
                using value_type = typename alloc_type::value_type;

                alloc_type _alloc;
                usize* _data = nullptr;

                template < template <typename> typename Alloc >
                friend class counter;                

            public:
                inline counter()
                requires (std::is_default_constructible_v<alloc_type>)
                {
                    _data = _alloc.allocate(1).unwrap();
                    *_data = 1;
                }

                inline counter(const alloc_type& alloc)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{alloc}
                {
                    _data = _alloc.allocate(1).unwrap();
                    *_data = 1;
                }

                inline counter(usize* ptr)
                requires (std::is_default_constructible_v<alloc_type>)
                    : _data{ptr}
                {
                    (*_data)++;
                }

                inline counter(const alloc_type& alloc, usize* ptr)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{alloc}, _data{ptr}
                {
                    (*_data)++;
                }

                inline counter(const counter& other)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{other._alloc}, _data{other._data}
                {
                    (*_data)++;
                }

                inline counter(const counter& other)
                requires (!std::is_copy_constructible_v<alloc_type>)
                    : _data{other._data}
                {
                    (*_data)++;
                }

                inline counter& operator=(const counter& rhs)
                {
                    _data = rhs._data;
                    (*_data)++;
                    return *this;
                }

                inline counter& operator=(counter&& rhs)
                {
                    _data = exchange(rhs._data, nullptr);
                    return *this;
                }

                inline usize& operator*()
                {
                    return *_data;
                }

                inline auto deallocate()
                {
                    return _alloc.deallocate(_data, 1);
                }

                inline usize get_size() const
                {
                    return 1u;
                }

                inline auto* get_pointer() const
                {
                    return _data;
                }

                inline void set_pointer(pointer_type ptr)
                {
                    _data = ptr;
                }
            };
            
        } // namespace shared_detail

        template < typename T, template <typename> typename Allocator = allocator >
        class shared_ptr
        {
        private:
            shared_detail::storage<T, Allocator> _value;
            shared_detail::counter<Allocator> _count{};

            template <typename U, template <typename> typename Alloc>
            friend class shared_ptr;

            inline void _delete()
            {
                if(_count.get_pointer() != nullptr)
                {
                    (*_count)--;

                    if(*_count == 0)
                    {
                        if (get() != nullptr) 
                        {
                            if constexpr (is_array<T>::value)
                            {
                                for (usize i = 0, size = _value.get_size(); i < size; ++i)
                                    _destroy_inplace(get()[size-i]);
                            }
                            else
                            {
                                _destroy_inplace(*get());
                            }
                        }
                        _value.deallocate().unwrap();
                        _count.deallocate().unwrap();
                    }

                    _value.set_pointer(nullptr);
                    _count.set_pointer(nullptr);
                }
            }

        public:
            using alloc_type = Allocator< remove_array_t<T> >;
            using pointer_type = typename alloc_type::pointer_type;
            using value_type = typename alloc_type::value_type;
            using reference_type = typename alloc_type::value_type&;

            inline shared_ptr() = default;
            inline shared_ptr(NullType) {}

            inline shared_ptr(pointer_type ptr) 
            requires (std::is_default_constructible_v<alloc_type>)
                : _value{ptr, 1u}
            {}

            inline shared_ptr(pointer_type ptr, usize size) 
            requires (std::is_default_constructible_v<alloc_type>)
                : _value{ptr, size}
            {}

            inline shared_ptr(const alloc_type& alloc) 
                : _value{alloc, 1u}, _count{alloc}
            {}

            inline shared_ptr(const alloc_type& alloc, usize size) 
                : _value{alloc, size}, _count{alloc}
            {}

            inline shared_ptr(pointer_type ptr, 
                const alloc_type& alloc, usize size) 
                : _value{ptr, alloc, size}, _count{alloc}
            {}

            template <typename U = T> 
            inline shared_ptr(const shared_ptr<U, Allocator>& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{other._value}, _count{other._count}
            {}

            template <typename U = T> 
            inline shared_ptr(shared_ptr<U, Allocator>&& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{move(other._value)}, _count{move(other._count)}
            {}

            inline ~shared_ptr()
            {
                _delete();
            }

            inline shared_ptr& operator=(NullType)
            {
                _delete();
                return *this;
            }

            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            inline shared_ptr& operator=(shared_ptr<U, Allocator>& rhs)
            {
                _delete();
                _value = rhs._value;
                _count = rhs._count;
                return *this;
            }

            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            inline shared_ptr& operator=(shared_ptr<U, Allocator>&& rhs)
            {
                _delete();
                _value = move(rhs._value);
                _count = move(rhs._count);
                return *this;
            }

            inline bool operator==(const shared_ptr& rhs) const
            {
                return get() == rhs.get();
            }

            inline bool operator!=(const shared_ptr& rhs) const
            {
                return get() == rhs.get();
            }

            inline bool operator==(NullType) const
            {
                return get() == nullptr;
            }

            inline bool operator!=(NullType) const
            {
                return get() == nullptr;
            }

            inline auto* get()
            {
                return _value.get_pointer();
            }

            inline auto* get() const
            {
                return _value.get_pointer();
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

            inline usize get_count()
            {
                return *_count;
            }

            inline bool is_unique()
            {
                return get_count() == 1;
            }
        };

        template < typename T, template <typename> typename Allocator >
        struct MakeShr
        {
            using single_object = shared_ptr<T, Allocator>;
        };

        template < typename T, template <typename> typename Allocator >
        struct MakeShr<T[], Allocator>
        {
            using array = shared_ptr<T[], Allocator>;
        };

        template < typename T, usize N, template <typename> typename Allocator >
        struct MakeShr<T[N], Allocator>
        {
            struct invalid_type {};  
        };

        template < typename T, template <typename> typename Allocator = allocator, typename... Args >
        requires (std::is_default_constructible_v<Allocator<uchar>>)
        static inline typename MakeShr<T, Allocator>::single_object 
        make_shared(Args&&... args)
        {
            Allocator<remove_array_t<T>> _alloc;
            auto* _ptr = _alloc.allocate(1).unwrap();
            _alloc.construct_at(_ptr, forward<Args>(args)...);
            return shared_ptr<T, Allocator>(_ptr);
        }

        template < typename T, template <typename> typename Allocator = allocator, typename U, typename... Args >
        static inline typename MakeShr<T, Allocator>::single_object 
        make_shared(Allocator<U>& alloc, Args&&... args)
        {
            auto* _ptr = static_cast<Allocator<remove_array_t<T>>>(alloc).allocate(1).unwrap();
            Allocator<remove_array_t<T>>::construct_at(_ptr, forward<Args>(args)...);
            return shared_ptr<T, Allocator>(_ptr, static_cast<Allocator<remove_array_t<T>>>(alloc), 1);
        }

        template < typename T, template <typename> typename Allocator = allocator >
        requires (std::is_default_constructible_v<Allocator<uchar>>)
        static inline typename MakeShr<T, Allocator>::array
        make_shared(usize size)
        {
            Allocator<remove_array_t<T>> _alloc;
            auto* _ptr = _alloc.allocate(size).unwrap();
            return shared_ptr<T, Allocator>(_ptr, size);
        }

        template < typename T, template <typename> typename Allocator = allocator, typename U >
        static inline typename MakeShr<T, Allocator>::array
        make_shared(Allocator<U>& alloc, usize size)
        {
            return shared_ptr<T, Allocator>(static_cast<Allocator<remove_array_t<T>>>(alloc), size);
        }

        template < typename T, template <typename> typename Allocator = allocator, typename... Args >
        static inline typename MakeShr<T, Allocator>::invalid_type 
        make_shared(Args&&...) = delete;
    }

    namespace atomic_types
    {
        namespace shared_detail
        {
            template <typename T, typename U>
            concept ConvertibleDerived = std::is_convertible_v<U, T> || std::is_base_of_v<T, U>;

            template < typename T, template <typename> typename Allocator >
            class storage
            {
            private:
                using alloc_type = Allocator<remove_array_t<T>>;
                using pointer_type = typename alloc_type::pointer_type;
                using value_type = typename alloc_type::value_type;

                alloc_type _alloc;
                pointer_type _data = nullptr;
                usize _size = 0;

                template <typename U, template <typename> typename Alloc>
                friend class storage;

            public:
                inline storage()
                requires (std::is_default_constructible_v<alloc_type>) = default;

                inline storage(pointer_type ptr, usize size)
                requires (std::is_default_constructible_v<alloc_type>)
                    : _data{ptr}, _size{size}
                {}

                inline storage(const alloc_type& alloc, usize size)
                requires (std::is_copy_constructible_v<value_type>)
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
                requires (std::is_copy_constructible_v<value_type>)
                    : _alloc{alloc}, _data{ptr}, _size{size}
                {}

                inline storage(const storage& other)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                inline storage(const storage& other)
                requires (!std::is_copy_constructible_v<alloc_type>)
                    : _data{other._data}, _size{other._size}
                {}

                inline storage(storage&& other)
                requires (std::is_move_constructible_v<alloc_type>)
                    : _alloc{move(other._alloc)}
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                inline storage(storage&& other)
                requires (!std::is_move_constructible_v<alloc_type>)
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                template <typename U = T>
                inline storage(const storage<U, Allocator>& other)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                template <typename U = T>
                inline storage(const storage<U, Allocator>& other)
                requires (!std::is_copy_constructible_v<alloc_type>)
                    : _data{other._data}, _size{other._size}
                {}

                template <typename U = T>
                inline storage(storage<U, Allocator>&& other)
                requires (std::is_move_constructible_v<alloc_type>)
                    : _alloc{move(other._alloc)}
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                template <typename U = T>
                inline storage(storage<U, Allocator>&& other)
                requires (!std::is_move_constructible_v<alloc_type>)
                {
                    _data = exchange(other._data, nullptr);
                    swap(_size, other._size);
                }

                inline storage& operator=(const storage& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                inline storage& operator=(storage&& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = exchange(rhs._data, nullptr);                    
                    swap(_size, rhs._size);
                    return *this;
                }

                template <typename U = T>
                inline storage& operator=(const storage<U, Allocator>& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                template <typename U = T>
                inline storage& operator=(storage<U, Allocator>&& rhs)
                {
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
                    this->_data = ptr;
                }

                inline void set_size(usize size)
                {
                    _size = size;
                }
            };

            template < template <typename> typename Allocator >
            class counter
            {
            private:
                using alloc_type = Allocator<atomic_usize>;
                using pointer_type = typename alloc_type::pointer_type;
                using value_type = typename alloc_type::value_type;

                alloc_type _alloc;
                atomic_usize* _data = nullptr;

                template < template <typename> typename Alloc >
                friend class counter;                

            public:
                inline counter()
                requires (std::is_default_constructible_v<alloc_type>)
                {
                    _data = _alloc.allocate(1).unwrap();
                    *_data = 1;
                }

                inline counter(const alloc_type& alloc)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{alloc}
                {
                    _data = _alloc.allocate(1).unwrap();
                    *_data = 1;
                }

                inline counter(atomic_usize* ptr)
                requires (std::is_default_constructible_v<alloc_type>)
                    : _data{ptr}
                {
                    (*_data)++;
                }

                inline counter(const alloc_type& alloc, atomic_usize* ptr)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{alloc}, _data{ptr}
                {
                    (*_data)++;
                }

                inline counter(const counter& other)
                requires (std::is_copy_constructible_v<alloc_type>)
                    : _alloc{other._alloc}, _data{other._data}
                {
                    (*_data)++;
                }

                inline counter(const counter& other)
                requires (!std::is_copy_constructible_v<alloc_type>)
                    : _data{other._data}
                {
                    (*_data)++;
                }

                inline counter& operator=(const counter& rhs)
                {
                    _data = rhs._data;
                    (*_data)++;
                    return *this;
                }

                inline counter& operator=(counter&& rhs)
                {
                    _data = exchange(rhs._data, nullptr);
                    return *this;
                }

                inline atomic_usize& operator*()
                {
                    return *_data;
                }

                inline auto deallocate()
                {
                    return _alloc.deallocate(_data, 1);
                }

                inline usize get_size() const
                {
                    return 1u;
                }

                inline auto* get_pointer() const
                {
                    return _data;
                }

                inline void set_pointer(pointer_type ptr)
                {
                    _data = ptr;
                }
            };
            
        } // namespace shared_detail

        template < typename T, template <typename> typename Allocator = allocator >
        class shared_ptr
        {
        private:
            shared_detail::storage<T, Allocator> _value;
            shared_detail::counter<Allocator> _count{};

            template <typename U, template <typename> typename Alloc>
            friend class shared_ptr;

            inline void _delete()
            {
                if(_count.get_pointer() != nullptr)
                {
                    (*_count)--;

                    if(*_count == 0)
                    {
                        if (get() != nullptr) 
                        {
                            if constexpr (is_array<T>::value)
                            {
                                for (usize i = 0, size = _value.get_size(); i < size; ++i)
                                    _destroy_inplace(get()[size-i]);
                            }
                            else
                            {
                                _destroy_inplace(*get());
                            }
                        }
                        _value.deallocate().unwrap();
                        _count.deallocate().unwrap();
                    }

                    _value.set_pointer(nullptr);
                    _count.set_pointer(nullptr);
                }
            }

        public:
            using alloc_type = Allocator< remove_array_t<T> >;
            using pointer_type = typename alloc_type::pointer_type;
            using value_type = typename alloc_type::value_type;
            using reference_type = typename alloc_type::value_type&;

            inline shared_ptr() = default;
            inline shared_ptr(NullType) {}

            inline shared_ptr(pointer_type ptr) 
            requires (std::is_default_constructible_v<alloc_type>)
                : _value{ptr, 1u}
            {}

            inline shared_ptr(pointer_type ptr, usize size) 
            requires (std::is_default_constructible_v<alloc_type>)
                : _value{ptr, size}
            {}

            inline shared_ptr(const alloc_type& alloc) 
                : _value{alloc, 1u}, _count{alloc}
            {}

            inline shared_ptr(const alloc_type& alloc, usize size) 
                : _value{alloc, size}, _count{alloc}
            {}

            inline shared_ptr(pointer_type ptr, 
                const alloc_type& alloc, usize size) 
                : _value{ptr, alloc, size}, _count{alloc}
            {}

            template <typename U = T> 
            inline shared_ptr(const shared_ptr<U, Allocator>& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{other._value}, _count{other._count}
            {}

            template <typename U = T> 
            inline shared_ptr(shared_ptr<U, Allocator>&& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{move(other._value)}, _count{move(other._count)}
            {}

            inline ~shared_ptr()
            {
                _delete();
            }

            inline shared_ptr& operator=(NullType)
            {
                _delete();
                return *this;
            }

            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            inline shared_ptr& operator=(shared_ptr<U, Allocator>& rhs)
            {
                _delete();
                _value = rhs._value;
                _count = rhs._count;
                return *this;
            }

            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            inline shared_ptr& operator=(shared_ptr<U, Allocator>&& rhs)
            {
                _delete();
                _value = move(rhs._value);
                _count = move(rhs._count);
                return *this;
            }

            inline bool operator==(const shared_ptr& rhs) const
            {
                return get() == rhs.get();
            }

            inline bool operator!=(const shared_ptr& rhs) const
            {
                return get() == rhs.get();
            }

            inline bool operator==(NullType) const
            {
                return get() == nullptr;
            }

            inline bool operator!=(NullType) const
            {
                return get() == nullptr;
            }

            inline auto* get()
            {
                return _value.get_pointer();
            }

            inline auto* get() const
            {
                return _value.get_pointer();
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

            inline usize get_count()
            {
                return *_count;
            }

            inline bool is_unique()
            {
                return get_count() == 1;
            }
        };

        template < typename T, template <typename> typename Allocator >
        struct MakeShr
        {
            using single_object = shared_ptr<T, Allocator>;
        };

        template < typename T, template <typename> typename Allocator >
        struct MakeShr<T[], Allocator>
        {
            using array = shared_ptr<T[], Allocator>;
        };

        template < typename T, usize N, template <typename> typename Allocator >
        struct MakeShr<T[N], Allocator>
        {
            struct invalid_type {};  
        };

        template < typename T, template <typename> typename Allocator = allocator, typename... Args >
        requires (std::is_default_constructible_v<Allocator<uchar>>)
        static inline typename MakeShr<T, Allocator>::single_object 
        make_shared(Args&&... args)
        {
            Allocator<remove_array_t<T>> _alloc;
            auto* _ptr = _alloc.allocate(1).unwrap();
            _alloc.construct_at(_ptr, forward<Args>(args)...);
            return shared_ptr<T, Allocator>(_ptr);
        }

        template < typename T, template <typename> typename Allocator = allocator, typename U, typename... Args> 
        static inline typename MakeShr<T, Allocator>::single_object 
        make_shared(Allocator<U>& alloc, Args&&... args)
        {
            auto* _ptr = static_cast<Allocator<remove_array_t<T>>>(alloc).allocate(1).unwrap();
            Allocator<remove_array_t<T>>::construct_at(_ptr, forward<Args>(args)...);
            return shared_ptr<T, Allocator>(_ptr, static_cast<Allocator<remove_array_t<T>>>(alloc), 1);
        }

        template < typename T, template <typename> typename Allocator = allocator >
        requires (std::is_default_constructible_v<Allocator<uchar>>)
        static inline typename MakeShr<T, Allocator>::array
        make_shared(usize size)
        {
            Allocator<remove_array_t<T>> _alloc;
            auto* _ptr = _alloc.allocate(size).unwrap();
            return shared_ptr<T, Allocator>(_ptr, size);
        }

        template < typename T, template <typename> typename Allocator = allocator, typename U >
        static inline typename MakeShr<T, Allocator>::array
        make_shared(Allocator<U>& alloc, usize size)
        {
            return shared_ptr<T, Allocator>(static_cast<Allocator<remove_array_t<T>>>(alloc), size);
        }

        template < typename T, template <typename> typename Allocator = allocator, typename... Args >
        static inline typename MakeShr<T, Allocator>::invalid_type 
        make_shared(Args&&...) = delete;
    }

    template < typename T, template <typename> typename Allocator = allocator >
    using unsafe_shared_ptr = non_atomic_types::shared_ptr<T, Allocator>;
    template < typename T, template <typename> typename Allocator = allocator >
    using safe_shared_ptr = atomic_types::shared_ptr<T, Allocator>;

    template < typename T, template <typename> typename Allocator = allocator, typename... Args >
    requires (std::is_default_constructible_v<Allocator<uchar>>)
    static inline auto make_unsafe_shared(Args&&... args)
    {
        return non_atomic_types::make_shared<T, Allocator, Args...>(forward<Args>(args)...);
    }

    template < typename T, template <typename> typename Allocator = allocator, typename... Args >
    requires (std::is_default_constructible_v<Allocator<uchar>>)
    static inline auto make_safe_shared(Args&&... args)
    {
        return atomic_types::make_shared<T, Allocator, Args...>(forward<Args>(args)...);
    }

    template < typename T, template <typename> typename Allocator = allocator, typename U, typename... Args >
    static inline auto make_unsafe_shared(Allocator<U>& alloc, Args&&... args)
    {
        return non_atomic_types::make_shared<T, Allocator, U, Args...>(alloc, forward<Args>(args)...);
    }

    template < typename T, template <typename> typename Allocator = allocator, typename U, typename... Args >
    static inline auto make_safe_shared(Allocator<U>& alloc, Args&&... args)
    {
        return atomic_types::make_shared<T, Allocator, U, Args...>(alloc, forward<Args>(args)...);
    }

    template < typename T, template <typename> typename Allocator = allocator >
    requires (std::is_default_constructible_v<Allocator<uchar>>)
    static inline auto make_unsafe_shared(usize size)
    {
        return non_atomic_types::make_shared<T, Allocator>(size);
    }

    template < typename T, template <typename> typename Allocator = allocator >
    requires (std::is_default_constructible_v<Allocator<uchar>>)
    static inline auto make_safe_shared(usize size)
    {
        return atomic_types::make_shared<T, Allocator>(size);
    }

    template < typename T, template <typename> typename Allocator = allocator, typename U >
    static inline auto make_unsafe_shared(Allocator<U>& alloc, usize size)
    {
        return non_atomic_types::make_shared<T, Allocator, U>(alloc, size);
    }

    template < typename T, template <typename> typename Allocator = allocator, typename U >
    static inline auto make_safe_shared(Allocator<U>& alloc, usize size)
    {
        return atomic_types::make_shared<T, Allocator, U>(alloc, size);
    }
}