#include <cassert>

#include <Containers/List.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Base/Result.hpp>

struct cache_entry
{
    int value;
    hsd::list<int>::iterator recency_position;
};

class lru_cache
{
    hsd::usize _capacity;
    hsd::list<int> _recency;
    hsd::unordered_map<int, cache_entry> _entries;

    void erase_key(int key)
    {
        auto position = _entries.begin();
        while (position != _entries.end() && position->first != key)
        {
            ++position;
        }
        assert(position != _entries.end());
        _entries.erase(position).unwrap();
    }

public:
    explicit lru_cache(hsd::usize capacity)
        : _capacity{capacity}
    {
        assert(capacity > 0);
    }

    void put(int key, int value)
    {
        auto found = _entries.at(key);
        if (found.is_ok())
        {
            auto& entry = found.unwrap().get();
            _recency.erase(entry.recency_position).unwrap();
            _recency.push_front(key);
            entry.recency_position = _recency.begin();
            entry.value = value;
            return;
        }

        if (_entries.size() == _capacity)
        {
            const int least_recent = _recency.back();
            _recency.pop_back();
            erase_key(least_recent);
        }

        _recency.push_front(key);
        _entries.emplace(key, cache_entry{value, _recency.begin()});
    }

    auto get(int key) -> hsd::result<int, hsd::runtime_error>
    {
        auto found = _entries.at(key);
        if (!found.is_ok())
        {
            return hsd::runtime_error{"Cache miss"};
        }

        auto& entry = found.unwrap().get();
        _recency.erase(entry.recency_position).unwrap();
        _recency.push_front(key);
        entry.recency_position = _recency.begin();
        return entry.value;
    }

    hsd::usize size() const
    {
        return _entries.size();
    }
};

int main()
{
    lru_cache cache{3};
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);

    assert(cache.get(1).unwrap() == 10);
    cache.put(4, 40);
    assert(!cache.get(2).is_ok());
    assert(cache.get(3).unwrap() == 30);
    assert(cache.get(4).unwrap() == 40);

    cache.put(1, 11);
    assert(cache.get(1).unwrap() == 11);
    assert(cache.size() == 3);

    cache.put(5, 50);
    assert(!cache.get(3).is_ok());
    assert(cache.get(1).unwrap() == 11);
    assert(cache.get(4).unwrap() == 40);
    assert(cache.get(5).unwrap() == 50);
}