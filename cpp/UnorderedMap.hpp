#pragma once

#include "Pair.hpp"
#include "Vector.hpp"
#include "Hash.hpp"
#include "Reference.hpp"

namespace hsd
{
    template< typename Key, typename T, typename Hasher = fnv1a<usize> >
    class unordered_map;

    namespace _detail
    {
        template< typename Key, typename T, typename Hasher = fnv1a<usize> >
        class map_value
        {
        private:
            usize _index = 0;
            pair<Key, T> _data;
            friend class unordered_map< Key, T, Hasher >;
        
        public:
            using value_type = T;
            using reference_type = T&;

            HSD_CONSTEXPR map_value(usize index, const Key& key, const value_type& val)
                : _index{index}, _data{key, val}
            {}

            HSD_CONSTEXPR map_value(usize index, Key&& key, value_type&& val)
                : _index{index}, _data{move(key), move(val)}
            {}

            constexpr pair<Key, T>& get() noexcept 
            { 
                return _data; 
            }
        };

        template< typename Key, typename T, typename Hasher = fnv1a<usize> >
        class iterator
        {
        private:
            using value_type = map_value< Key, T, Hasher >;
            value_type* _it;

        public:
            HSD_CONSTEXPR iterator(value_type* iter)
                : _it{iter}
            {}

            constexpr iterator& operator=(const iterator& rhs)
            {
                _it = rhs._it;
                return *this;
            }

            constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            {
                return lhs._it == rhs._it;
            }

            constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs)
            {
                return lhs._it != rhs._it;
            }

            constexpr iterator& operator++()
            {
                _it++;
                return *this;
            }

            constexpr iterator& operator--()
            {
                _it--;
                return *this;
            }

            constexpr iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            constexpr iterator operator--(i32)
            {
                iterator tmp = *this;
                operator--();
                return tmp;
            }

            constexpr pair<Key, T>& operator*()
            {
                return _it->get();
            }

            constexpr pair<Key, T>& operator*() const
            {
                return _it->get();
            }
        };
    } // namespace _detail

    template< typename Key, typename T, typename Hasher >
    class unordered_map
    {
    private:
        using map_value_type = _detail::map_value< Key, T, Hasher >;
        vector< vector<reference<map_value_type>> > _buckets;
        static constexpr f64 _limit_ratio = 0.75f;
        vector<map_value_type> _data;

        HSD_CONSTEXPR void _replace()
        {
            usize _new_size = _buckets.size() * 2;
            _buckets.clear();
            _buckets.resize(_new_size);

            for(map_value_type& _val : _data)
            {
                auto _hash_rez = Hasher::get_hash(_val._data.first);
                usize _index = _hash_rez % _new_size;
                _buckets[_index].emplace_back(_val);
            }
        }

        constexpr usize _get(const Key& key)
        {
            auto _key_hash = Hasher::get_hash(key);
            usize _index = _key_hash % _buckets.size();

            for(map_value_type& _val : _buckets[_index])
            {
                if(_key_hash == Hasher::get_hash(_val._data.first))
                {
                    return _val._index;
                }
            }

            return -1;
        }

    public:
        using reference_type = T&;
        using iterator = _detail::iterator< Key, T, Hasher >;
        using const_iterator = typename vector<map_value_type>::const_iterator;

        HSD_CONSTEXPR ~unordered_map() = default;

        HSD_CONSTEXPR unordered_map()
            : _buckets(10)
        {}

        HSD_CONSTEXPR unordered_map(const unordered_map& other)
            : _buckets{other._buckets.size()}, _data{other._data}
        {
            for(auto& _val : _data)
            {
                auto _hash_rez = Hasher::get_hash(_val._data.first);
                usize _index = _hash_rez % _buckets.size();
                _buckets[_index].emplace_back(_val);
            }
        }

        HSD_CONSTEXPR unordered_map(unordered_map&& other)
            : _buckets{move(other._buckets)}, _data{move(other._data)}
        {}

        HSD_CONSTEXPR reference_type operator[](const Key& key) noexcept
        {
            return (*emplace(move(key)).first).second;
        }

        HSD_CONSTEXPR reference_type operator[](const Key& key) const noexcept
        {
            return (*emplace(move(key)).first).second;
        }

        HSD_CONSTEXPR reference_type at(const Key& key)
        {
            usize _data_index = _get(key);

            if(_data_index == static_cast<usize>(-1))
            {
                throw std::out_of_range("");
            }

            return _data[_data_index]._data.second;
        }

        HSD_CONSTEXPR reference_type at(const Key& key) const
        {
            usize _data_index = _get(key);

            if(_data_index == static_cast<usize>(-1))
            {
                throw std::out_of_range("");
            }

            return _data[_data_index]._data.second;
        }

        template< typename NewKey, typename... Args >
        HSD_CONSTEXPR pair<iterator, bool> emplace(NewKey&& key, Args&&... args)
        {
            usize _data_index = _get(key);

            if(_data_index != static_cast<usize>(-1))
            {
                return {_data.begin() + _data_index, false};
            }
            else
            {
                _data.emplace_back(_data.size(), move(key), move(T{forward<Args>(args)...}));

                if(static_cast<f64>(_data.size()) / _buckets.size() >= _limit_ratio)
                    _replace();

                return {_data.end() - 1, true};
            }
        }

        constexpr iterator begin()
        {
            return _data.begin();
        }

        constexpr iterator end()
        {
            return _data.end();
        }

        constexpr const_iterator cbegin() const
        {
            return _data.begin();
        }

        constexpr const_iterator cend() const
        {
            return _data.end();
        }
    };
} // namespace hsd