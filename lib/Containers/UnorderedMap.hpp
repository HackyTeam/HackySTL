#pragma once

/// @file UnorderedMap.hpp
/// @brief Allocator-aware hash map with explicit bucket storage.
/// @details Stores values and bucket indices separately, rehashes as needed,
/// preserves duplicate-key semantics, and propagates one allocator everywhere.

#include "../Wrappers/Pair.hpp"
#include "Vector.hpp"
#include "../Base/Hash.hpp"

namespace hsd
{
    namespace umap_detail
    {
        /// @brief Error returned when a key is absent from the map.
        struct bad_key
        {
            /// @brief Returns the missing-key diagnostic.
            /// @return A stable diagnostic string.
            constexpr const char* pretty_error() const
            {
                return "Tried to use an invalid key";
            }
        };

        /// @brief Error returned when an iterator does not identify an element.
        struct bad_access
        {
            /// @brief Returns the invalid-access diagnostic.
            /// @return A stable diagnostic string.
            constexpr const char* pretty_error() const
            {
                return "Tried to access an element out of bounds";
            }
        };
    } // namespace umap_detail

    /// @brief Allocator-aware hash map with separate value and bucket storage.
    template <typename Key, typename T, typename Hasher = hash<usize, Key>, typename Allocator = allocator>
    class unordered_map
    {
    private:
        /// @brief Initial number of hash buckets; always a power of two.
        static constexpr usize _initial_bucket_count = 16;
        /// @brief Ends a bucket chain and marks "key not found".
        static constexpr usize _npos = ~static_cast<usize>(0);

        static constexpr usize _top_bit = ~(_npos >> 1);
        /// @brief Hash value type produced by `Hasher`.
        using hash_type = typename Hasher::ResultType;
        /// @brief Index storage for bucket heads.
        using index_vector = vector<usize, Allocator>;

        /// @brief Per-record chain metadata kept next to each other so one
        ///        cache line answers both "is this the key?" and "what next?".
        struct _link_node
        {
            /// @brief Hash computed when the record was inserted.
            hash_type hash;
            /// @brief Next record index in the same bucket chain, or `_npos`.
            usize next;

            /// @brief Creates the chain metadata of one record.
            /// @param new_hash Hash of the record's key.
            /// @param new_next Next record in the bucket chain.
            constexpr _link_node(hash_type new_hash, usize new_next)
                : hash{new_hash}, next{new_next}
            {}
        };

        /// @brief Chain metadata storage parallel to the dense records.
        using link_vector = vector<_link_node, Allocator>;
        /// @brief Contiguous key/value storage.
        using data_vector = vector<pair<Key, T>, Allocator>;
        /// @brief Allocator traits for map entries.
        using alloc_traits = allocator_traits<Allocator, pair<Key, T>>;

        /// @brief Maximum entry count before the next bucket expansion.
        static constexpr usize _load_limit(usize bucket_count)
        {
            return bucket_count / 4 * 3;
        }

        /// @brief Smallest power-of-two bucket count that holds `entries`
        ///        without exceeding the load limit.
        /// @param entries Number of records the table must hold.
        /// @return Bucket count; the top power of two if `entries` is so large
        ///         that the following allocation is expected to fail.
        static constexpr usize _bucket_count_for(usize entries)
        {
            usize _count = _initial_bucket_count;

            while (_load_limit(_count) < entries)
            {
                if (_count == _top_bit)
                {
                    return _count;
                }

                _count <<= 1;
            }

            return _count;
        }

        /// @brief First record index of each bucket chain, or `_npos`.
        /// @details Its size is the bucket count and is a power of two.
        index_vector _heads;
        /// @brief Cached hash and chain link of each record.
        /// @details Parallel to `_data`; the cached hashes let a rebuild skip
        /// re-hashing keys.
        link_vector _links;
        /// @brief Contiguous key/value records owned by the map.
        data_vector _data;

        /// @brief Maps a hash to its bucket.
        /// @param hash Hash of a key.
        /// @return Bucket index in `[0, _heads.size())`.
        constexpr usize _bucket_of(hash_type hash) const
        {
            // Fibonacci hashing: the top log2(buckets) bits of hash * 2^w/phi.
            // Every input bit reaches those bits, so identity hashes of
            // sequential, strided or high-bit-only integer keys still spread
            // evenly. The bucket count is a power of two of at least 16.
            constexpr unsigned _bits = sizeof(usize) * 8;
            constexpr usize _golden = sizeof(usize) >= 8
                ? static_cast<usize>(0x9E3779B97F4A7C15ULL)
                : static_cast<usize>(0x9E3779B9U);
            const auto _log2_buckets = static_cast<unsigned>(
                countr_zero(static_cast<usize>(_heads.size()))
            );

            return (static_cast<usize>(hash) * _golden) >> (_bits - _log2_buckets);
        }

        /// @brief Pushes record `index` onto the front of its bucket chain.
        /// @param index Record index whose cached hash selects the bucket.
        constexpr void _link(usize index)
        {
            auto& _node = _links[index];
            const usize _bucket = _bucket_of(_node.hash);
            _node.next = _heads[_bucket];
            _heads[_bucket] = index;
        }

        /// @brief Rebuilds every bucket chain for `bucket_count` buckets.
        /// @details Uses the cached hashes, so keys are not hashed again and
        /// no allocation happens beyond the head array itself.
        /// @param bucket_count Requested bucket count; a power of two.
        constexpr void _replace(usize bucket_count)
        {
            _heads.clear();
            _heads.reserve(bucket_count);

            for (usize _index = 0; _index < bucket_count; _index++)
            {
                _heads.emplace_back(_npos);
            }

            for (usize _index = 0; _index < _data.size(); _index++)
            {
                _link(_index);
            }
        }

        /// @brief Registers the record most recently appended to `_data`.
        /// @param hash Hash of the new record's key.
        constexpr void _append(hash_type hash)
        {
            _links.emplace_back(hash, _npos);

            if (_data.size() > _load_limit(_heads.size()))
            {
                _replace(_heads.size() * 2);
            }
            else
            {
                _link(_data.size() - 1);
            }
        }

        /// @brief Finds a record by key and precomputed hash.
        /// @param key Key searched for.
        /// @param hash Hash of `key`.
        /// @return Record index, or `_npos` when the key is absent.
        constexpr usize _find(const Key& key, hash_type hash) const
        {
            usize _index = _heads[_bucket_of(hash)];

            while (_index != _npos)
            {
                const auto& _node = _links[_index];

                if (_node.hash == hash && _data[_index].first == key)
                    return _index;

                _index = _node.next;
            }

            return _npos;
        }

        /// @brief Finds a record by key.
        /// @param key Key searched for.
        /// @return Record index, or all-bits-one when the key is absent.
        constexpr usize _get(const Key& key) const
        {
            return _find(key, Hasher::get_hash(key));
        }

    public:
        /// @brief Mapped value reference type.
        using reference_type = T&;
        /// @brief Mutable iterator over stored key/value records.
        using iterator = typename data_vector::iterator;
        /// @brief Const iterator over stored key/value records.
        using const_iterator = typename data_vector::const_iterator;

        constexpr ~unordered_map() {}

        /// @brief Constructs an empty map with a default-constructed allocator.
        constexpr unordered_map()
        requires (DefaultConstructible<Allocator>)
            : unordered_map{Allocator{}}
        {}

        ///
        /// @brief Constructs an empty map using the supplied allocator.
        ///
        /// The same allocator instance is propagated to the value storage,
        /// bucket storage, and every nested bucket vector.
        ///
        /// @param alloc - Allocator used by all storage owned by the map.
        ///
        constexpr unordered_map(const Allocator& alloc)
            : _heads(alloc), _links(alloc), _data(alloc)
        {
            _replace(_initial_bucket_count);
        }

        /// @brief Copies values and selects a copy-construction allocator.
        /// @param other Map whose entries are copied.
        constexpr unordered_map(const unordered_map& other)
            : unordered_map{
                alloc_traits::select_on_container_copy_construction(
                    other.get_allocator()
                )
            }
        {
            for (const auto& _value : other._data)
            {
                emplace(_value.first, _value.second);
            }
        }

        /// @brief Transfers value and bucket storage from another map.
        /// @details The moved-from map is left empty with a usable bucket
        /// table, since moving out of `_heads` alone would leave no buckets.
        /// @param other Map whose storage is moved.
        constexpr unordered_map(unordered_map&& other)
            : _heads{hsd::move(other._heads)}, _links{hsd::move(other._links)},
            _data{hsd::move(other._data)}
        {
            other._replace(_initial_bucket_count);
        }

        template <usize N>
        /// @brief Constructs a map from copied entries with a default allocator.
        /// @param other Source entries copied into the map.
        constexpr unordered_map(const pair<Key, T> (&other)[N])
        requires (DefaultConstructible<Allocator>)
            : unordered_map{other, Allocator{}}
        {}

        ///
        /// @brief Constructs a map from copied entries and an allocator.
        ///
        /// @tparam N - Number of source entries.
        /// @param other - Entries copied into the map.
        /// @param alloc - Allocator used by all map storage.
        ///
        template <usize N>
        constexpr unordered_map(
            const pair<Key, T> (&other)[N], const Allocator& alloc)
            : unordered_map{alloc}
        {
            for (usize _index = 0; _index < N; _index++)
            {
                emplace(
                    other[_index].first, 
                    other[_index].second
                );
            }
        }

        template <usize N>
        /// @brief Constructs a map from moved entries with a default allocator.
        /// @param other Source entries moved into the map.
        constexpr unordered_map(pair<Key, T> (&&other)[N])
        requires (DefaultConstructible<Allocator>)
            : unordered_map{hsd::move(other), Allocator{}}
        {}

        ///
        /// @brief Constructs a map from moved entries and an allocator.
        ///
        /// @tparam N - Number of source entries.
        /// @param other - Entries moved into the map.
        /// @param alloc - Allocator used by all map storage.
        ///
        template <usize N>
        constexpr unordered_map(
            pair<Key, T> (&&other)[N], const Allocator& alloc)
            : unordered_map{alloc}
        {
            for (usize _index = 0; _index < N; _index++)
            {
                emplace(
                    hsd::move(other[_index].first), 
                    hsd::move(other[_index].second)
                );
            }
        }

        /// @brief Replaces this map by exchanging complete allocator/storage state.
        /// @param rhs Map whose storage is exchanged.
        /// @return `*this` after the exchange.
        constexpr unordered_map& operator=(unordered_map&& rhs)
        {
            if (this != &rhs)
            {
                hsd::swap(_heads, rhs._heads);
                hsd::swap(_links, rhs._links);
                hsd::swap(_data, rhs._data);
            }

            return *this;
        }

        /// @brief Copy-assigns through a complete temporary map state.
        /// @param rhs Map whose entries are copied.
        /// @return `*this` after replacement.
        constexpr unordered_map& operator=(const unordered_map& rhs)
        {
            if (this == &rhs) return *this;

            unordered_map _copy{rhs};
            hsd::swap(_heads, _copy._heads);
            hsd::swap(_links, _copy._links);
            hsd::swap(_data, _copy._data);

            return *this;
        }

        template <usize N>
        /// @brief Copy-assigns entries from an array and returns this map.
        /// @param rhs Source entries copied into the map.
        /// @return `*this` after replacement.
        constexpr unordered_map& operator=(const pair<Key, T>(&rhs)[N])
        {
            clear();

            for (usize _index = 0; _index < N; _index++)
                emplace(rhs[_index].first, rhs[_index].second);

            return *this;
        }

        template <usize N>
        /// @brief Move-assigns entries from an array and returns this map.
        /// @param rhs Source entries moved into the map.
        /// @return `*this` after replacement.
        constexpr unordered_map& operator=(pair<Key, T>(&&rhs)[N])
        {
            clear();

            for (usize _index = 0; _index < N; _index++)
            {
                emplace(
                    hsd::move(rhs[_index].first), 
                    hsd::move(rhs[_index].second)
                );
            }

            return *this;
        }

        /// @brief Finds or default-inserts a mutable mapped value.
        /// @param key Key looked up or inserted.
        /// @return Mutable mapped value for `key`.
        constexpr auto& operator[](const Key& key)
        {
            return emplace(key).first->second;
        }

        /// @brief Finds or default-inserts using a forwarded key.
        /// @param key Key looked up or inserted.
        /// @return Mutable mapped value for `key`.
        constexpr auto& operator[](Key&& key)
        {
            return emplace(hsd::forward<Key>(key)).first->second;
        }

        /// @brief Returns const access to an existing mapped value.
        /// @param key Key looked up.
        /// @return Const mapped value; missing keys terminate through `unwrap`.
        constexpr const auto& operator[](const Key& key) const
        {
            return at(key).unwrap().get();
        }

        /// @brief Returns checked mutable lookup for `key`.
        /// @param key Key searched for.
        /// @return Reference result or `bad_key` when absent.
        constexpr auto at(const Key& key)
            -> result<reference<T>, umap_detail::bad_key>
        {
            usize _data_index = _get(key);

            if (_data_index == static_cast<usize>(-1))
                return umap_detail::bad_key{};

            return {_data[_data_index].second};
        }

        /// @brief Returns checked const lookup for `key`.
        /// @param key Key searched for.
        /// @return Const reference result or `bad_key` when absent.
        constexpr auto at(const Key& key) const
            -> result<reference<const T>, umap_detail::bad_key>
        {
            usize _data_index = _get(key);

            if (_data_index == static_cast<usize>(-1))
                return umap_detail::bad_key{};

            return {_data[_data_index].second};
        }

        /// @brief Inserts a copied key and constructed mapped value if absent.
        /// @param key Key copied into a new record when absent.
        /// @param args Arguments used to construct the mapped value.
        /// @return Iterator to the record and whether insertion occurred.
        template< typename NewKey, typename... Args >
        constexpr pair<iterator, bool> emplace(const NewKey& key, const Args&... args)
        {
            const Key& _lookup_key = key;
            const auto _hash = Hasher::get_hash(_lookup_key);
            const usize _data_index = _find(_lookup_key, _hash);

            if (_data_index != _npos)
            {
                return {_data.begin() + _data_index, false};
            }
            else
            {
                _data.emplace_back(key, T{args...});
                _append(_hash);

                return {_data.end() - 1, true};
            }
        }

        /// @brief Inserts a forwarded key without moving from lvalue keys.
        /// @param key Key forwarded into a new record when absent.
        /// @param args Arguments used to construct the mapped value.
        /// @return Iterator to the record and whether insertion occurred.
        template< typename NewKey, typename... Args >
        constexpr pair<iterator, bool> emplace(NewKey&& key, Args&&... args)
        {
            const Key& _lookup_key = key;
            const auto _hash = Hasher::get_hash(_lookup_key);
            const usize _data_index = _find(_lookup_key, _hash);

            if (_data_index != _npos)
            {
                return {_data.begin() + _data_index, false};
            }
            else
            {
                _data.emplace_back(
                    hsd::forward<NewKey>(key), hsd::move(T{hsd::forward<Args>(args)...})
                );
                _append(_hash);

                return {_data.end() - 1, true};
            }
        }

        /// @brief Erases the element at `pos` and rebuilds bucket indices.
        /// @param pos Iterator identifying the record to erase.
        /// @return Iterator to the next record, or `bad_access` for an invalid position.
        constexpr auto erase(const_iterator pos)
            -> result<iterator, umap_detail::bad_access>
        {
            const usize _index = static_cast<usize>(pos - _data.cbegin());

            auto _new_iter = _data
                .erase(pos)
                .unwrap();

            // Unlink the record from its bucket chain.
            const usize _bucket = _bucket_of(_links[_index].hash);

            if (_heads[_bucket] == _index)
            {
                _heads[_bucket] = _links[_index].next;
            }
            else
            {
                usize _previous = _heads[_bucket];

                while (_links[_previous].next != _index)
                {
                    _previous = _links[_previous].next;
                }

                _links[_previous].next = _links[_index].next;
            }

            // Chains are strictly descending: a record is always linked in
            // front of older (lower-index) ones, and rebuilds relink in
            // ascending order. So `next` always points below its own record,
            // records before `_index` are unaffected, and later records only
            // need renumbering when they point past the erased position. The
            // shift and the renumbering share one pass, with no hashing.
            const usize _last = _links.size() - 1;

            for (usize _from = _index; _from < _last; ++_from)
            {
                auto _node = _links[_from + 1];
                _node.next -= static_cast<usize>(
                    (_node.next != _npos) & (_node.next > _index)
                );
                _links[_from] = _node;
            }

            _links.pop_back();

            for (auto& _head : _heads)
            {
                _head -= static_cast<usize>(
                    (_head != _npos) & (_head > _index)
                );
            }

            return _new_iter;
        }

        /// @brief Removes all values while retaining map bucket storage.
        constexpr void clear()
        {
            _data.clear();
            _links.clear();

            for (auto& _head : _heads)
            {
                _head = _npos;
            }
        }

        /// @brief Reserves storage for at least `expected_size` entries.
        /// @param expected_size Expected number of key/value pairs.
        /// @note May invalidate iterators when value storage or buckets grow.
        constexpr void reserve(usize expected_size)
        {
            _data.reserve(expected_size);
            _links.reserve(expected_size);

            const usize _new_bucket_count = _bucket_count_for(expected_size);

            if (_new_bucket_count > _heads.size())
            {
                _replace(_new_bucket_count);
            }
        }

        /// @brief Returns the number of stored key/value pairs.
        /// @return Number of records currently stored.
        constexpr usize size() const
        {
            return _data.size();
        }

        ///
        /// @brief Returns the allocator associated with all map storage.
        ///
        /// @brief Returns the allocator shared by values and buckets.
        /// @return Allocator associated with map storage.
        constexpr Allocator get_allocator() const
        {
            return _data.get_allocator();
        }

        /// @brief Returns a mutable iterator to the first record.
        /// @return Mutable begin iterator.
        constexpr iterator begin()
        {
            return _data.begin();
        }

        /// @brief Returns a const iterator to the first record.
        /// @return Const begin iterator.
        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        /// @brief Returns a mutable iterator past the final record.
        /// @return Mutable end iterator.
        constexpr iterator end()
        {
            return _data.end();
        }

        /// @brief Returns a const iterator past the final record.
        /// @return Const end iterator.
        constexpr const_iterator end() const
        {
            return cend();
        }

        /// @brief Returns a const iterator to the first record.
        /// @return Const begin iterator.
        constexpr const_iterator cbegin() const
        {
            return _data.cbegin();
        }

        /// @brief Returns a const iterator past the final record.
        /// @return Const end iterator.
        constexpr const_iterator cend() const
        {
            return _data.cend();
        }

        /// @brief Returns a mutable reverse iterator to the final record.
        /// @return Mutable reverse-begin iterator.
        constexpr auto rbegin()
        {
            return _data.rbegin();
        }

        /// @brief Returns a const reverse iterator to the final record.
        /// @return Const reverse-begin iterator.
        constexpr auto rbegin() const
        {
            return crbegin();
        }

        /// @brief Returns a mutable reverse iterator before the first record.
        /// @return Mutable reverse-end iterator.
        constexpr auto rend()
        {
            return _data.rend();
        }

        /// @brief Returns a const reverse iterator before the first record.
        /// @return Const reverse-end iterator.
        constexpr auto rend() const
        {
            return crend();
        }

        /// @brief Returns a const reverse iterator to the final record.
        /// @return Const reverse-begin iterator.
        constexpr auto crbegin() const
        {
            return _data.crbegin();
        }

        /// @brief Returns a const reverse iterator before the first record.
        /// @return Const reverse-end iterator.
        constexpr auto crend() const
        {
            return _data.crend();
        }
    };

    /// @brief Deduction guide for moved entry arrays with the default allocator.
    /// @param other Entry array used to deduce key and mapped types.
    template < typename Key, typename T, usize N >
    unordered_map(pair<Key, T> (&&other)[N]) 
        -> unordered_map<Key, T, hash<usize, Key>>;

    /// @brief Deduction guide for copied entry arrays with the default allocator.
    /// @param other Entry array used to deduce key and mapped types.
    template < typename Key, typename T, usize N >
    unordered_map(const pair<Key, T> (&other)[N]) 
        -> unordered_map<Key, T, hash<usize, Key>>;

    /// @brief Deduction guide for moved entry arrays with an explicit allocator.
    /// @param other Entry array used to deduce key and mapped types.
    /// @param alloc Allocator used by the resulting map.
    template < typename Key, typename T, usize N, typename Allocator >
    unordered_map(
        pair<Key, T> (&&other)[N], const Allocator& alloc
    ) -> unordered_map<Key, T, hash<usize, Key>, Allocator>;

    /// @brief Deduction guide for copied entry arrays with an explicit allocator.
    /// @param other Entry array used to deduce key and mapped types.
    /// @param alloc Allocator used by the resulting map.
    template < typename Key, typename T, usize N, typename Allocator >
    unordered_map(
        const pair<Key, T> (&other)[N], const Allocator& alloc
    ) -> unordered_map<Key, T, hash<usize, Key>, Allocator>;

    /// @brief Buffered-allocator unordered_map alias.
    template < typename Key, typename T >
    using buffered_umap = unordered_map<
        Key, T, hash<usize, Key>, buffered_allocator
    >;
} // namespace hsd