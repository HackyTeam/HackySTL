#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>

struct CollideAll
{
    using ResultType = hsd::usize;

    static constexpr hsd::usize get_hash(const int&)
    {
        return 7;
    }
};

struct IdentityHash
{
    using ResultType = hsd::usize;

    static constexpr hsd::usize get_hash(const int& key)
    {
        return static_cast<hsd::usize>(key);
    }
};

struct IdentityHash64
{
    using ResultType = hsd::usize;

    static constexpr hsd::usize get_hash(const std::uint64_t& key)
    {
        return static_cast<hsd::usize>(key);
    }
};

template <typename Map, typename Key, typename KeyGen, typename Convert>
static void run(unsigned seed, long steps, KeyGen make_key, Convert convert)
{
    std::mt19937_64 rng(seed);
    Map map;
    std::unordered_map<Key, int> reference;

    auto verify = [&]
    {
        assert(map.size() == reference.size());

        for (const auto& [key, value] : reference)
        {
            auto found = map.at(convert(key));
            assert(found.is_ok() && found.unwrap().get() == value);
        }

        std::size_t visited = 0;

        for (auto it = map.begin(); it != map.end(); ++it)
        {
            ++visited;
        }

        assert(visited == reference.size());
    };

    for (long step = 0; step < steps; ++step)
    {
        const int op = static_cast<int>(rng() % 100);
        const Key key = make_key(rng);
        const int value = static_cast<int>(rng() % 100000);

        if (step < steps * 6 / 10)
        {
            // Growth phase: mostly inserts.
            if (op < 85)
            {
                const auto inserted = map.emplace(convert(key), value);
                const auto fresh = reference.emplace(key, value).second;
                assert(inserted.second == fresh);
            }
            else
            {
                auto found = map.at(convert(key));
                assert(found.is_ok() == (reference.find(key) != reference.end()));
            }
        }
        else if (op < 35)
        {
            const auto inserted = map.emplace(convert(key), value);
            const auto fresh = reference.emplace(key, value).second;
            assert(inserted.second == fresh);
        }
        else if (op < 50)
        {
            map[convert(key)] = value;
            reference[key] = value;
        }
        else if (op < 62)
        {
            auto found = map.at(convert(key));
            const auto expected = reference.find(key);
            assert(found.is_ok() == (expected != reference.end()));

            if (found.is_ok())
            {
                assert(found.unwrap().get() == expected->second);
            }
        }
        else if (op < 90)
        {
            if (!reference.empty())
            {
                // Erase a random element: step the iterator to a random position.
                const auto skip = static_cast<std::size_t>(rng() % reference.size());
                auto it = map.begin();

                for (std::size_t i = 0; i < skip; ++i)
                {
                    ++it;
                }

                const auto erased_key = it->first;
                const auto removed = map.erase(it);
                assert(removed.is_ok());

                bool matched = false;

                for (auto ref_it = reference.begin(); ref_it != reference.end(); ++ref_it)
                {
                    if (convert(ref_it->first) == erased_key)
                    {
                        reference.erase(ref_it);
                        matched = true;
                        break;
                    }
                }

                assert(matched);
            }
        }
        else if (op < 91 && rng() % 40 == 0)
        {
            map.clear();
            reference.clear();
        }
        else if (op < 94)
        {
            map.reserve(rng() % 3000);
        }
        else if (op < 97)
        {
            Map copy{map};
            Map moved{hsd::move(copy)};
            map = moved;
        }

        if (step % 503 == 0)
        {
            verify();
        }
    }

    verify();
}

int main()
{
    const unsigned seed = 1;

    run<hsd::unordered_map<int, int>, int>(
        seed,
        12000,
        [](auto& rng) { return static_cast<int>(rng() % 30000); },
        [](int key) { return key; });

    // Multiples of 4096 share their low 12 bits, as do aligned addresses.
    run<hsd::unordered_map<int, int, IdentityHash>, int>(
        seed,
        6000,
        [](auto& rng) { return static_cast<int>((rng() % 20000) * 4096); },
        [](int key) { return key; });

    // Keys that differ only above bit 32.
    run<hsd::unordered_map<std::uint64_t, int, IdentityHash64>, std::uint64_t>(
        seed,
        6000,
        [](auto& rng) { return static_cast<std::uint64_t>(rng() % 5000) << 40; },
        [](std::uint64_t key) { return key; });

    // Every key collides: one long chain.
    run<hsd::unordered_map<int, int, CollideAll>, int>(
        seed,
        2000,
        [](auto& rng) { return static_cast<int>(rng() % 300); },
        [](int key) { return key; });

    run<hsd::unordered_map<hsd::string, int>, std::string>(
        seed,
        2500,
        [](auto& rng)
        {
            return std::string(30 + (rng() % 5), static_cast<char>('a' + rng() % 26))
                + std::to_string(rng() % 20000);
        },
        [](const std::string& text) { return hsd::string(text.c_str()); });

    return 0;
}
