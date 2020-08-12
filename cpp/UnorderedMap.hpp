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
        template< typename Key, typename T, typename Hasher = fnv1a<uint64_t> >
        class bucket
        {
        private:
            struct bucket_impl
            {
                pair<Key, T> _value;
                bucket_impl* _next = nullptr;

                constexpr bucket_impl() {}

                constexpr bucket_impl(const T& val)
                    : _value{val}
                {}

                constexpr bucket_impl(T&& val)
                {
                    _value = hsd::move(val);
                }
            };
            
            bucket_impl *_iterator = nullptr;
            friend class unordered_map< Key, T, Hasher >;

            constexpr friend bool operator==(const bucket& lhs, const bucket& rhs)
            {
                return lhs._iterator == rhs._iterator;
            }

            constexpr void linear_search(const Key& hash_key)
            {
                auto _hash_res = Hasher::get_hash(hash_key);

                for(; _iterator != nullptr; _iterator = _iterator->_next)
                    if(_hash_res == Hasher::get_hash(_iterator->_value.first))
                        break;
            }

            constexpr pair<Key, T>* get()
            {
                return &_iterator->_value;
            }

            constexpr  pair<Key, T>* get() const
            {
                return &_iterator->_value;
            }

            constexpr void push_front(const T& value)
            {
                if(_iterator == nullptr)
                {
                    _iterator = new bucket_impl(value);
                }
                else
                {
                    bucket_impl* _element = new bucket_impl(value);
                    _element->_next = _iterator;
                    _iterator = _element;
                }
            }

            constexpr void push_front(T&& value)
            {
                if(_iterator == nullptr)
                {
                    _iterator = new bucket_impl(hsd::move(value));
                }
                else
                {
                    bucket_impl* _element = new bucket_impl(hsd::move(value));
                    _element->_next = _iterator;
                    _iterator = _element;
                }
            }

            constexpr void push_front(const bucket& other)
            {
                if(_iterator == nullptr)
                {
                    _iterator =  other._iterator;
                    _iterator->_next = nullptr;
                }
                else
                {
                    other._iterator->_next = _iterator;
                    _iterator =  other._iterator;
                }
            }

            template<typename... Args>
            constexpr void emplace_front(Args&&... args)
            {
                if(_iterator == nullptr)
                {
                    _iterator = new bucket_impl();
                    _iterator->_value.~pair<Key, T>();
                    new (&_iterator->_value) pair<Key, T>(hsd::forward<Args>(args)...);
                }
                else
                {
                    bucket_impl* _element = new bucket_impl();
                    _element->_next = _iterator;
                    _element->_value.~pair<Key, T>();
                    new (&_element->_value) pair<Key, T>(hsd::forward<Args>(args)...);
                    _iterator = _element;
                }
            }

            constexpr void pop_front()
            {
                bucket_impl* _element = _iterator;
                _iterator = _iterator->_next;
                delete _element;
            }
        public:
        
            constexpr bucket() {}
            constexpr bucket(hsd::nullptr_t) {}

            constexpr bucket(const bucket& other)
            {
                _iterator = other._iterator;
            }

            constexpr bucket(bucket&& other)
            {
                _iterator = other._iterator;
                other._iterator = nullptr;
            }

            constexpr bucket& operator=(const bucket& rhs)
            {
                _iterator = rhs._iterator;
                return *this;
            }

            constexpr bucket& operator=(bucket&& rhs)
            {
                _iterator = rhs._iterator;
                rhs._iterator = nullptr;
                return *this;
            }

            constexpr bucket& operator=(nullptr_t)
            {
                _iterator = nullptr;
                return *this;
            }

            constexpr friend bool operator!=(const bucket& lhs, const bucket& rhs)
            {
                return lhs._iterator != rhs._iterator;
            }

            constexpr bucket& operator++()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                _iterator = _iterator->_next;
                return *this;
            }

            constexpr bucket operator++(int)
            {
                bucket tmp = *this;
                operator++();
                return tmp;
            }

            constexpr pair<Key, T>& operator*()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return _iterator->_value;
            }

            constexpr pair<Key, T>& operator*() const
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return _iterator->_value;
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
        using iterator = unordered_map_detail::bucket< Key, T, Hasher >;
        using bucket = unordered_map_detail::bucket< Key, T, Hasher >;
        using hash_table = vector<bucket>;

        static constexpr float _limit_ratio = 0.75f;
        iterator _map_iter = nullptr;
        size_t _table_size = 10;
        size_t _size = 0;
        hash_table _table;
        

        constexpr pair<Key, T>* _get(const Key& hash_key)
        {
            auto _hash_result = Hasher::get_hash(hash_key);
            size_t _index = _hash_result % _table_size;

            for(auto _it = _table[_index]; _it != end(); _it++)
            {
                if(_hash_result == Hasher::get_hash(_it->first))
                    return _it.get();
            }

            return nullptr;
        }

        constexpr void destroy_iter(iterator& iter)
        {
            for(; iter != end(); iter.pop_front());
        }

    public:
        constexpr unordered_map()
        {
            _table.resize(_table_size);
        }
        
        constexpr ~unordered_map()
        {
            destroy_iter(_map_iter);
        }

        constexpr unordered_map(unordered_map&& map)
            : _map_iter{hsd::move(map._map_iter)}, _table_size{map._table_size}, _size{map._size}, _table{hsd::move(map._table)}
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

            if(_val == nullptr)
            {
                T _insert_val;
                return emplace(hash_key, _insert_val).first->second;
            }
            else
                return _val->second;
        }

        constexpr void reserve(size_t table_size)
        {
            hash_table _tmp = hsd::move(_table);
            auto _tmp_node = _map_iter; 
            _map_iter = nullptr;
            _table_size = table_size;
            _table.resize(_table_size);

            for(auto _it = _tmp_node ; _it != end(); _it++)
                emplace(hsd::move(_it->first), hsd::move(_it->second));

            destroy_iter(_tmp_node);
        }

        template<typename... Args>
        constexpr pair<iterator, bool> emplace(Args&&... args)
        {
            auto _key = make_tuple(hsd::forward<Args>(args)...).template get<0>();
            auto _index = Hasher::get_hash(_key) % _table_size;
            pair<Key, T>* _res = _get(_key);
            
            if(_res == nullptr)
            {
                _size++;
                _table[_index].emplace_front(hsd::forward<Args>(args)...);
                _map_iter.push_front(_table[_index]);
                
                if(static_cast<float>(_size) / _table_size >= _limit_ratio)
                    reserve(_table_size *= 2);

                return {_map_iter, true};
            }

            iterator _it = _map_iter;
            _it.linear_search(_key);
            return {_it, false};
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

        constexpr iterator end()
        {
            return {nullptr};
        }

        constexpr const iterator cbegin() const
        {
            return begin();
        }

        constexpr const iterator cend() const
        {
            return end();
        }
    };
} // namespace hsd
