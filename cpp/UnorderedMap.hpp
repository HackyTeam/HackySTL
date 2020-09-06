#pragma once

#include "Vector.hpp"
#include "Pair.hpp"
#include "Hash.hpp"
#include "Tuple.hpp"

namespace hsd
{
    template< typename Key, typename T, typename Hasher = fnv1a<uint64_t> > class unordered_map;

    namespace unordered_map_detail
    {
        template< typename Key, typename T, typename Hasher = fnv1a<uint64_t> > class iterator;

        template< typename Key, typename T, typename Hasher = fnv1a<uint64_t> >
        class bucket
        {
        private:
            struct bucket_impl
            {
                pair<Key, T>* _value = nullptr;
                bucket_impl* _next = nullptr;

                constexpr bucket_impl() {}
            };
            
            bucket_impl *_bucket = nullptr;

        public:
            constexpr bucket() {}
            constexpr bucket(hsd::nullptr_t) {}

            constexpr bucket(const bucket& other)
            {
                _bucket = other._bucket;
            }

            constexpr bucket(bucket&& other)
            {
                _bucket = other._bucket;
                other._bucket = nullptr;
            }

            constexpr bucket& operator=(const bucket& rhs)
            {
                _bucket = rhs._bucket;
                return *this;
            }

            constexpr bucket& operator=(bucket&& rhs)
            {
                _bucket = rhs._bucket;
                rhs._bucket = nullptr;
                return *this;
            }

            constexpr bucket& operator=(nullptr_t)
            {
                _bucket = nullptr;
                return *this;
            }

            constexpr friend bool operator!=(const bucket& lhs, const bucket& rhs)
            {
                return lhs._bucket != rhs._bucket;
            }

            constexpr friend bool operator==(const bucket& lhs, const bucket& rhs)
            {
                return lhs._bucket == rhs._bucket;
            }

            constexpr pair<Key, T>* operator->()
            {
                if(_bucket == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }

            constexpr pair<Key, T>* operator->() const
            {
                if(_bucket == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }

            constexpr void increment()
            {
                if(_bucket == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                _bucket = _bucket->_next;
            }

            constexpr pair<Key, T>* get()
            {
                return _bucket->_value;
            }

            constexpr  pair<Key, T>* get() const
            {
                return _bucket->_value;
            }

            constexpr void push_front(const pair<Key, T>& value)
            {
                if(_bucket == nullptr)
                {
                    _bucket = new bucket_impl;
                    _bucket->_value = new pair<Key, T>(value);
                }
                else
                {
                    bucket_impl* _element = new bucket_impl;
                    _element->_value = new pair<Key, T>(value);
                    _element->_next = _bucket;
                    _bucket = _element;
                }
            }

            constexpr void push_front(pair<Key, T>&& value)
            {
                if(_bucket == nullptr)
                {
                    _bucket = new bucket_impl;
                    _bucket->_value = new pair<Key, T>(hsd::move(value));
                }
                else
                {
                    bucket_impl* _element = new bucket_impl;
                    _element->_value = new pair<Key, T>(hsd::move(value));
                    _element->_next = _bucket;
                    _bucket = _element;
                }
            }

            constexpr void push_front(const iterator<Key, T, Hasher>& iter)
            {
                if(_bucket == nullptr)
                {
                    _bucket = new bucket_impl;
                    _bucket->_value = iter.operator->();
                }
                else
                {
                    bucket_impl* _element = new bucket_impl;
                    _element->_value = iter.operator->();
                    _element->_next = _bucket;
                    _bucket = _element;
                }
            }

            template<typename... Args>
            constexpr void emplace_front(Args&&... args)
            {
                if(_bucket == nullptr)
                {
                    _bucket = new bucket_impl();
                    _bucket->_value = new pair<Key, T>(hsd::forward<Args>(args)...);
                }
                else
                {
                    bucket_impl* _element = new bucket_impl();
                    _element->_next = _bucket;
                    _element->_value = new pair<Key, T>(hsd::forward<Args>(args)...);
                    _bucket = _element;
                }
            }

            constexpr void destroy()
            {
                for(; _bucket != nullptr; pop_front());
            }

            constexpr void clear()
            {
                bucket_impl* _element;
                while(_bucket != nullptr)
                {
                    _element = _bucket;
                    _bucket = _bucket->_next;
                    delete _element;
                }
            }

            constexpr void pop_front()
            {
                bucket_impl* _element = _bucket;
                _bucket = _bucket->_next;
                delete _element->_value;
                delete _element;
            }
        };

        template< typename Key, typename T, typename Hasher >
        class iterator
        {
        private:
            struct iterator_impl
            {
                pair<Key, T>* _value = nullptr;
                iterator_impl* _next = nullptr;

                constexpr iterator_impl() {}

                constexpr iterator_impl(pair<Key, T>* value)
                    : _value{value}
                {}
            };
            
            iterator_impl *_iterator = nullptr;
            friend class unordered_map< Key, T, Hasher >;

            constexpr iterator() {}
            constexpr iterator(hsd::nullptr_t) {}

            constexpr iterator(const iterator& other)
            {
                _iterator = other._iterator;
            }

            constexpr iterator(iterator&& other)
            {
                _iterator = other._iterator;
                other._iterator = nullptr;
            }

            constexpr void push_front(const bucket<Key, T, Hasher>& bkt)
            {
                if(_iterator == nullptr)
                {
                    _iterator = new iterator_impl(bkt.get());
                }
                else
                {
                    iterator_impl* _element = new iterator_impl(bkt.get());
                    _element->_next = _iterator;
                    _iterator = _element;
                }
            }

            constexpr void destroy()
            {
                for(; _iterator != nullptr; pop_front());
            }

            constexpr void pop_front()
            {
                iterator_impl* _element = _iterator;
                _iterator = _iterator->_next;
                delete _element;
            }

            constexpr void linear_search(const Key& hash_key)
            {
                auto _hash_res = Hasher::get_hash(hash_key);

                for(; _iterator != nullptr; _iterator = _iterator->_next)
                    if(_hash_res == Hasher::get_hash(_iterator->_value->first))
                        return;
            }

            constexpr pair<Key, T>* get()
            {
                return _iterator->_value;
            }

            constexpr  pair<Key, T>* get() const
            {
                return _iterator->_value;
            }

        public:
            constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator != rhs._iterator;
            }

            constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator == rhs._iterator;
            }

            constexpr iterator& operator++()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                _iterator = _iterator->_next;
                return *this;
            }

            constexpr iterator operator++(int)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            constexpr pair<Key, T>& operator*()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return *get();
            }

            constexpr pair<Key, T>& operator*() const
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return *get();
            }

            constexpr pair<Key, T>* operator->()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }

            constexpr pair<Key, T>* operator->() const
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }
        };
        
    } // namespace unordered_map_detail
    
    template< typename Key, typename T, typename Hasher >
    class unordered_map
    {
    private:
        using iterator = unordered_map_detail::iterator< Key, T, Hasher >;
        using bucket = unordered_map_detail::bucket< Key, T, Hasher >;
        using hash_table = vector<bucket>;

        static constexpr float _limit_ratio = 0.75f;
        iterator _map_iter = nullptr;
        size_t _table_size = 10;
        size_t _size = 0;
        hash_table _table;
        

        constexpr pair<size_t, bucket> _get(const Key& hash_key)
        {
            auto _hash_result = Hasher::get_hash(hash_key);
            size_t _index = _hash_result % _table_size;

            for(auto _it = _table[_index]; _it != nullptr; _it.increment())
            {
                if(_hash_result == Hasher::get_hash(_it->first))
                    return {_index, _it};
            }

            return {_index, nullptr};
        }

        constexpr void _replace(const iterator& iter)
        {
            auto _res_index = Hasher::get_hash(iter->first) % _table_size;
            _table[_res_index].push_front(iter);
        }

        constexpr void _reserve(size_t table_size)
        {
            clear_table();
            _table_size = table_size;
            _table.resize(_table_size);

            for(auto _it = _map_iter ; _it != end(); _it++)
                _replace(_it);
        }

        constexpr void destroy_table()
        {
            for(auto& _iter : _table)
                _iter.destroy();
        }

        constexpr void clear_table()
        {
            for(auto& _iter : _table)
            {
                _iter.clear();
                _iter = nullptr;
            }
        }

        template<typename... Args>
        constexpr pair<bucket, bool> _emplace(size_t index, Args&&... args)
        {
            _size++;
            
            if(static_cast<float>(_size) / _table_size > _limit_ratio)
            {
                _reserve(_table_size *= 2);
                auto _key = make_tuple(hsd::forward<Args>(args)...).template get<0>();
                size_t _new_index = Hasher::get_hash(_key) % _table_size;

                return _emplace(_new_index, hsd::forward<Args>(args)...);
            }
            else
            {
                _table[index].emplace_front(hsd::forward<Args>(args)...);
            }

            return {_table[index], true};
        }

    public:
        using const_iterator = const iterator;

        constexpr unordered_map()
        {
            _table.resize(_table_size);
        }
        
        constexpr ~unordered_map()
        {
            destroy_table();
            _map_iter.destroy();
        }

        constexpr unordered_map(unordered_map&& map)
            : _table_size{map._table_size}, _size{map._size}, _table{hsd::move(map._table)}
        {}

        constexpr unordered_map(const unordered_map& map)
            : _table_size{map._table_size}, _size{map._size}, _table{map._table}
        {
            for(auto _bucket : _table)
                for(auto _it = _bucket; _it != end(); _it++)
                    _map_iter.push_front(_it);
        }

        constexpr T& operator[](const Key& hash_key)
        {
            auto _val = _get(hash_key);

            if(_val.second == nullptr)
            {
                T _insert_val;
                auto&& [_iter, _] = _emplace(_val.first, hsd::move(hash_key), hsd::move(_insert_val));
                _map_iter.push_front(_iter);
                return _iter->second;
            }
            else
                return _val.second->second;
        }

        template<typename... Args>
        constexpr pair<bucket, bool> emplace(Args&&... args)
        {
            auto _key = make_tuple(hsd::forward<Args>(args)...).template get<0>();
            auto&& [_res_index, _res_iter] = _get(_key);
            
            if(_res_iter == nullptr)
            { 
                auto&& [_iter, _] = _emplace(_res_index, hsd::forward<Args>(args)...);
                _map_iter.push_front(_iter);
                
                return {_iter, true};
            }

            return {_res_iter, false};
        }

        constexpr size_t size()
        {
            return _size;
        }

        constexpr bool empty()
        {
            return size() == 0;
        }

        constexpr iterator begin()
        {
            return _map_iter;
        }

        constexpr iterator begin() const
        {
            return _map_iter;
        }

        constexpr iterator end()
        {
            return {nullptr};
        }

        constexpr iterator end() const
        {
            return {nullptr};
        }

        constexpr const_iterator cbegin()
        {
            return begin();
        }

        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        constexpr const_iterator cend()
        {
            return end();
        }

        constexpr const_iterator cend() const
        {
            return end();
        }
    };
} // namespace hsd
