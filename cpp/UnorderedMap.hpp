#pragma once

#include "Result.hpp"
#include "Pair.hpp"
#include "Vector.hpp"
#include "Hash.hpp"

namespace hsd
{
    template< typename Key, typename T, typename Hasher = fnv1a<usize>, 
        template <typename> typename Allocator = allocator >
    class unordered_map;

    namespace umap_detail
    {
        template < template <typename> typename Allocator >
        concept DefaultAlloc = std::is_default_constructible_v<Allocator<uchar>>;
        template < template <typename> typename Allocator >
        concept CopyAlloc = std::is_copy_constructible_v<Allocator<uchar>>;

        struct bad_key
        {
            const char* operator()() const
            {
                return "Tried to use an invalid key";
            }
        };
    } // namespace umap_detail

    template< typename Key, typename T, typename Hasher, 
        template <typename> typename Allocator >
    class unordered_map
    {
    private:
        using ref_value = pair< typename Hasher::ResultType, usize >;
        using ref_vector = vector< ref_value, Allocator >;

        static constexpr f64 _limit_ratio = 0.75f;
        vector< ref_vector, Allocator > _buckets;
        vector< pair<Key, T>, Allocator > _data;

        HSD_CONSTEXPR void _replace()
        {
            usize _new_size = _buckets.size() * 2;
            _buckets.clear();
            _buckets.resize(_new_size);

            for(usize _index = 0; _index < _data.size(); _index++)
            {
                auto _hash_rez = Hasher::get_hash(_data[_index].first);
                usize _bucket_index = _hash_rez % _new_size;
                _buckets[_bucket_index].emplace_back(_hash_rez, _index);
            }
        }

        constexpr pair<usize, usize> _get(const Key& key) const
        {
            auto _key_hash = Hasher::get_hash(key);
            usize _index = _key_hash % _buckets.size();

            for(auto& _val : _buckets[_index])
            {
                if(_key_hash == _val.first)
                {
                    return {_val.second, _index};
                }
            }

            return {static_cast<usize>(-1), _index};
        }

    public:
        using reference_type = T&;
        using iterator = typename vector<pair<Key, T>>::iterator;
        using const_iterator = typename vector<pair<Key, T>>::const_iterator;

        HSD_CONSTEXPR ~unordered_map() {}

        HSD_CONSTEXPR unordered_map()
        requires (umap_detail::DefaultAlloc<Allocator>)
            : _buckets(10)
        {}

        HSD_CONSTEXPR unordered_map(const Allocator<uchar>& alloc)
        requires (
            !umap_detail::DefaultAlloc<Allocator> || 
            umap_detail::CopyAlloc<Allocator>
        )
            : _buckets(10, static_cast<Allocator<ref_vector>>(alloc)), _data(alloc)
        {}

        HSD_CONSTEXPR unordered_map(const unordered_map& other)
            : _buckets{other._buckets.size()}, _data{other._data}
        {
            for(auto& _val : _data)
            {
                auto _hash_rez = Hasher::get_hash(_val.get().first);
                usize _index = _hash_rez % _buckets.size();
                _buckets[_index].emplace_back(_hash_rez, _index);
            }
        }

        HSD_CONSTEXPR unordered_map(unordered_map&& other)
            : _buckets{move(other._buckets)}, _data{move(other._data)}
        {}

        template <usize N>
        HSD_CONSTEXPR unordered_map(pair<Key, T> (&&other)[N])
        requires (umap_detail::DefaultAlloc<Allocator>)
            : _buckets(10)
        {
            for(usize _index = 0; _index < N; _index++)
            {
                emplace(
                    move(other[_index].first), 
                    move(other[_index].second)
                );
            }
        }

        HSD_CONSTEXPR unordered_map& operator=(unordered_map&& rhs)
        {
            _buckets = move(rhs._buckets);
            _data = move(rhs._data);
            return *this;
        }

        HSD_CONSTEXPR unordered_map& operator=(const unordered_map& rhs)
        {
            clear();

            for(pair<Key, T>& val : rhs._data)
                emplace(val.first, val.second);

            return *this;
        }

        template <usize N>
        HSD_CONSTEXPR unordered_map& operator=(const pair<Key, T>(&rhs)[N])
        {
            clear();

            for(usize _index = 0; _index < N; _index++)
            {
                emplace(rhs[_index].first, rhs[_index].second);
            }

            return *this;
        }

        template <usize N>
        HSD_CONSTEXPR unordered_map& operator=(pair<Key, T>(&&rhs)[N])
        {
            clear();

            for(usize _index = 0; _index < N; _index++)
            {
                emplace(
                    move(rhs[_index].first), 
                    move(rhs[_index].second)
                );
            }

            return *this;
        }

        HSD_CONSTEXPR auto& operator[](const Key& key) noexcept
        {
            return emplace(key).first->second;
        }

        HSD_CONSTEXPR const auto& operator[](const Key& key) const
        {
            return at(key).unwrap();
        }

        HSD_CONSTEXPR auto at(const Key& key)
            -> Result< reference<T>, umap_detail::bad_key >
        {
            usize _data_index = _get(key).first;

            if(_data_index == static_cast<usize>(-1))
                return umap_detail::bad_key{};

            return {_data[_data_index].second};
        }

        HSD_CONSTEXPR auto at(const Key& key) const
            -> Result< reference<const T>, umap_detail::bad_key >
        {
            usize _data_index = _get(key).first;

            if(_data_index == static_cast<usize>(-1))
                return umap_detail::bad_key{};

            return {_data[_data_index].second};
        }

        template< typename NewKey, typename... Args >
        HSD_CONSTEXPR pair<iterator, bool> emplace(const NewKey& key, const Args&... args)
        {
            auto [_data_index, _bucket_index] = _get(key);

            if(_data_index != static_cast<usize>(-1))
            {
                return {_data.begin() + _data_index, false};
            }
            else
            {
                _data.emplace_back(key, T{forward<Args>(args)...});

                if(static_cast<f64>(_data.size()) / static_cast<f64>(_buckets.size()) >= _limit_ratio)
                { 
                    _replace();
                }
                else
                {
                    _buckets[_bucket_index].emplace_back(Hasher::get_hash(_data.back().first), _data.size() - 1);
                }

                return {_data.end() - 1, true};
            }
        }

        template< typename NewKey, typename... Args >
        HSD_CONSTEXPR pair<iterator, bool> emplace(NewKey&& key, Args&&... args)
        {
            auto [_data_index, _bucket_index] = _get(key);

            if(_data_index != static_cast<usize>(-1))
            {
                return {_data.begin() + _data_index, false};
            }
            else
            {
                _data.emplace_back(move(key), move(T{forward<Args>(args)...}));

                if(static_cast<f64>(_data.size()) / static_cast<f64>(_buckets.size()) >= _limit_ratio)
                {
                    _replace();
                }
                else
                {
                    _buckets[_bucket_index].emplace_back(Hasher::get_hash(_data.back().first), _data.size() - 1);
                }

                return {_data.end() - 1, true};
            }
        }

        HSD_CONSTEXPR void clear()
        {
            _data.clear();
            _buckets.clear();
        }

        constexpr iterator begin()
        {
            return _data.begin();
        }

        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        constexpr iterator end()
        {
            return _data.end();
        }

        constexpr const_iterator end() const
        {
            return cend();
        }

        constexpr const_iterator cbegin() const
        {
            return _data.cbegin();
        }

        constexpr const_iterator cend() const
        {
            return _data.cend();
        }
    };

    template< typename Key, typename T, usize N >
    unordered_map(pair<Key, T> (&&other)[N]) 
        -> unordered_map< Key, T, fnv1a<usize>, allocator >;
} // namespace hsd