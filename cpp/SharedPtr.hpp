#pragma once

#include "Utility.hpp"

namespace hsd
{
    namespace non_atomic_types
    {
        namespace shared_detail
        {
            template <typename T, typename U>
            concept Derived = std::is_base_of_v<T, U>;
            template <typename T>
            concept ArrayPointer = is_array<T>::value;
            template <typename T>
            concept ValuePointer = !ArrayPointer<T>;

            template <typename T>
            struct ptr_info
            {
                remove_array_t<T>* _value = nullptr;
                usize* _size = nullptr;
            };

            template <typename T>
            struct deleter
            {
                void operator()(ptr_info<T>& ptr) requires(ValuePointer<T>)
                {
                    if(*ptr._size == 1)
                    {
                        delete ptr._value;
                        delete ptr._size;
                        ptr._value = nullptr;
                        ptr._size = nullptr; 
                    }
                    else
                    {
                        ptr._value = nullptr;
                        (*ptr._size)--;
                        ptr._size = nullptr;
                    }
                }

                void operator()(ptr_info<T>& ptr) requires(ArrayPointer<T>)
                {
                    if(*ptr._size == 1)
                    {
                        delete[] ptr._value;
                        delete ptr._size;
                        ptr._value = nullptr;
                        ptr._size = nullptr;
                    }
                    else
                    {
                        ptr._value = nullptr;
                        (*ptr._size)--;
                    }
                }
            };

            template <typename T, typename Deleter>
            struct storage : private Deleter
            {
                shared_detail::ptr_info<T> _ptr;

                constexpr Deleter& get_deleter()
                {
                    return *this;
                }
            };
        } // namespace shared_detail

        template < typename T, typename Deleter = shared_detail::deleter<T> >
        class shared_ptr
        {
        private:
            shared_detail::storage<T, Deleter> _value;

            template <typename U, typename Del>
            friend class shared_ptr;

            HSD_CONSTEXPR void _delete()
            {
                _value.get_deleter()(_value._ptr);
            }

        public:
            template <typename U>
            using pointer = U*;

            template <typename U>
            using reference = U&;

            template <typename U>
            using remove_array_pointer = remove_array_t<U>*;

            shared_ptr() = default;
            HSD_CONSTEXPR shared_ptr(NullType) {}

            HSD_CONSTEXPR shared_ptr(pointer<T> ptr) requires(shared_detail::ValuePointer<T>)
            {
                _value._ptr._value = ptr;
                _value._ptr._size = new usize(1);
            }

            HSD_CONSTEXPR shared_ptr(remove_array_pointer<T> ptr) requires(shared_detail::ArrayPointer<T>)
            {
                _value._ptr = ptr;
                _value._ptr._size = new usize(1);
            }

            constexpr shared_ptr(const shared_ptr& ptr)
            {
                _value = ptr._value;
                (*_value._ptr._size)++;
            }

            constexpr shared_ptr(shared_ptr&& ptr)
            {
                _value = ptr._value;
                ptr._value._ptr._size = nullptr;
                ptr._value._ptr._value = nullptr;
            }

            template <typename U> requires(shared_detail::Derived<T, U>)
            constexpr shared_ptr(const shared_ptr<U>& ptr)
            {
                _value = ptr._value;
                (*_value._ptr._size)++;
            }

            template <typename U> requires(shared_detail::Derived<T, U>)
            constexpr shared_ptr(shared_ptr<U>&& ptr)
            {
                _value = ptr._value;
                ptr._value._ptr._size = nullptr;
                ptr._value._ptr._value = nullptr;
            }

            HSD_CONSTEXPR ~shared_ptr()
            {
                _delete();
            }

            HSD_CONSTEXPR shared_ptr& operator=(NullType)
            {
                _delete();
                return *this;
            }

            HSD_CONSTEXPR shared_ptr& operator=(shared_ptr& lhs)
            {
                _delete();
                _value = lhs._value;
                (*_value._ptr._size)++;
                return *this;
            }

            HSD_CONSTEXPR shared_ptr& operator=(shared_ptr&& lhs)
            {
                _delete();
                _value = lhs._value;
                lhs._value._ptr._size = nullptr;
                lhs._value._ptr._value = nullptr;
                return *this;
            }

            template <typename U> requires(shared_detail::Derived<T, U>)
            HSD_CONSTEXPR shared_ptr& operator=(shared_ptr<U>& lhs)
            {
                _delete();
                _value = lhs._value;
                (*_value._ptr._size)++;
                return *this;
            }

            template <typename U> requires(shared_detail::Derived<T, U>)
            HSD_CONSTEXPR shared_ptr& operator=(shared_ptr<U>&& lhs)
            {
                _delete();
                _value = lhs._value;
                lhs._value._ptr._size = nullptr;
                lhs._value._ptr._value = nullptr;
                return *this;
            }

            constexpr remove_array_pointer<T> get()
            {
                return _value._ptr._value;
            }

            constexpr remove_array_pointer<T> get() const
            {
                return _value._ptr._value;
            }

            constexpr pointer<T> operator->()
            {
                return get();
            }

            constexpr pointer<T> operator->() const
            {
                return get();
            }

            constexpr reference<T> operator*()
            {
                return *get();
            }

            constexpr reference<T> operator*() const
            {
                return *get();
            }

            constexpr usize get_size()
            {
                return *_value._ptr_size;
            }

            constexpr bool is_unique()
            {
                return get_size() == 1;
            }
        };

        template <typename T>
        struct MakeShr
        {
            using single_object = shared_ptr<T>;
        };

        template <typename T>
        struct MakeShr<T[]>
        {
            using array = shared_ptr<T[]>;
        };

        template <typename T, usize N>
        struct MakeShr<T[N]>
        {
            struct invalid_type {};  
        };

        template <typename T, typename... Args>
        static HSD_CONSTEXPR typename MakeShr<T>::single_object 
        make_shared(Args&&... args)
        {
            return shared_ptr<T>(new T(hsd::forward<Args>(args)...));
        }

        template <typename T>
        static HSD_CONSTEXPR typename MakeShr<T>::array 
        make_shared(usize size)
        {
            using ptr_type = remove_array_t<T>;
            return shared_ptr<T>(new ptr_type[size]{});
        }

        template <typename T, typename... Args>
        static HSD_CONSTEXPR typename MakeShr<T>::invalid_type 
        make_shared(Args&&...) = delete;
    }
}