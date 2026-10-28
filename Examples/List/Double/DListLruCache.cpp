// Title: LRU cache
// Level: Advanced
// Shows: a doubly linked list plus an unordered_map as a least-recently-used cache.

#include <Containers/List.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// The list orders keys from most to least recently used, and the map finds a
// key's list node in O(1). List nodes never move, so the stored iterators stay
// valid while other nodes are added or removed.
class lru_cache
{
    struct entry
    {
        hsd::i32 value;
        hsd::list<hsd::i32>::iterator position;
    };

    hsd::usize _capacity;
    hsd::list<hsd::i32> _recency;
    hsd::unordered_map<hsd::i32, entry> _entries;

    void erase_entry(hsd::i32 key)
    {
        // The map erases by iterator, so find the record first.
        for (auto iter = _entries.begin(); iter != _entries.end(); ++iter)
        {
            if (iter->first == key)
            {
                _entries.erase(iter).unwrap();
                return;
            }
        }
    }

public:
    hsd::usize hits = 0;
    hsd::usize misses = 0;
    hsd::usize evictions = 0;

    explicit lru_cache(hsd::usize capacity)
        : _capacity{capacity}
    {}

    // Moves the key to the front of the recency list.
    void touch(entry& item, hsd::i32 key)
    {
        _recency.erase(item.position).unwrap();
        _recency.push_front(key);
        item.position = _recency.begin();
    }

    bool get(hsd::i32 key, hsd::i32& out)
    {
        auto found = _entries.at(key);

        if (found.is_ok() == false)
        {
            ++misses;
            return false;
        }

        auto& item = found.unwrap().get();
        touch(item, key);
        out = item.value;
        ++hits;
        return true;
    }

    void put(hsd::i32 key, hsd::i32 value)
    {
        auto found = _entries.at(key);

        if (found.is_ok())
        {
            auto& item = found.unwrap().get();
            item.value = value;
            touch(item, key);
            return;
        }

        if (_entries.size() == _capacity)
        {
            // The back of the list is the least recently used key.
            const hsd::i32 oldest = _recency.back();
            _recency.pop_back();
            erase_entry(oldest);
            ++evictions;
        }

        _recency.push_front(key);
        _entries.emplace(key, entry{value, _recency.begin()});
    }

    void print_order() const
    {
        hsd::print("order:"_fmt);
        for (const auto& key : _recency)
        {
            hsd::print(" {}"_fmt, key);
        }
        hsd::println(""_fmt);
    }
};

int main()
{
    lru_cache cache{3};

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.print_order();

    hsd::i32 value = 0;
    if (cache.get(1, value))
    {
        hsd::println("get 1 -> {}"_fmt, value);
    }
    cache.print_order();

    // The cache is full, so inserting 4 evicts the least recent key, which is 2.
    cache.put(4, 40);
    cache.print_order();
    hsd::println("get 2 hit: {}"_fmt, cache.get(2, value) ? "true" : "false");

    // Updating an existing key refreshes it without evicting anything.
    cache.put(3, 33);
    cache.print_order();

    // Replay a longer access pattern on a fresh cache and report the totals.
    lru_cache replay{3};
    for (hsd::i32 step = 0; step < 40; ++step)
    {
        const hsd::i32 key = (step * step) % 7;
        if (replay.get(key, value) == false)
        {
            replay.put(key, key * 100);
        }
    }

    replay.print_order();
    hsd::println("hits {} misses {} evictions {}"_fmt,
        replay.hits, replay.misses, replay.evictions);

    return 0;
}
