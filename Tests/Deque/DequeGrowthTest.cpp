#include <Containers/Deque.hpp>
#include <Containers/String.hpp>
#include <cassert>
#include <deque>
#include <random>
#include <string>

struct Pod
{
    int a;
    char b;
    double c;

    bool operator==(const Pod&) const = default;
};

static void check(const hsd::deque<Pod>& d, const std::deque<Pod>& e)
{
    assert(d.size() == e.size());
    size_t i = 0;

    for (auto it = d.begin(); it != d.end(); ++it, ++i)
    {
        assert(*it == e[i]);
    }
}

int main()
{
    std::mt19937 rng(99);
    long grows = 0;

    for (int round = 0; round < 8; ++round)
    {
        hsd::deque<Pod> d;
        std::deque<Pod> e;

        // 0 balanced, 1 push_front heavy, 2 push_back heavy, 3 queue-like.
        const int bias = round % 4;

        for (long step = 0; step < 8000; ++step)
        {
            int op = rng() % 100;

            Pod p = {
                static_cast<int>(rng()),
                static_cast<char>(rng()),
                static_cast<double>(rng() % 1000) / 7.0
            };

            auto cap_before = d.capacity();

            if (bias == 1 && op >= 40 && op < 60)
            {
                op = 0;
            }

            if (bias == 2 && op >= 40 && op < 60)
            {
                op = 20;
            }

            if (bias == 3)
            {
                if (op < 50)
                {
                    op = 20;
                }
                else if (op < 98)
                {
                    op = 45;
                }
            }

            if (op < 20)
            {
                d.push_front(p);
                e.push_front(p);
            }
            else if (op < 40)
            {
                d.push_back(p);
                e.push_back(p);
            }
            else if (op < 55)
            {
                if (!e.empty())
                {
                    d.pop_front();
                    e.pop_front();
                }
            }
            else if (op < 70)
            {
                if (!e.empty())
                {
                    d.pop_back();
                    e.pop_back();
                }
            }
            else if (op < 85)
            {
                if (bias != 3 && (step % 500) == 0)
                {
                    size_t n = e.size() + rng() % 300;
                    d.resize(n);
                    e.resize(n, Pod{});
                }
            }
            else if (op < 92)
            {
                d.reserve(rng() % 4000);
            }
            else if (op < 96)
            {
                hsd::deque<Pod> c = d;
                check(c, e);
            }
            else if (!e.empty())
            {
                assert(d.front() == e.front());
                assert(d.back() == e.back());
            }

            if (d.capacity() != cap_before)
            {
                ++grows;
            }

            if (step % 211 == 0 || d.size() != e.size())
            {
                check(d, e);
            }
        }

        check(d, e);
    }

    // The runs must really grow the ring while it holds wrapped contents.
    assert(grows > 40);

    // Owning elements take the element-wise path through the same growth code.
    {
        std::mt19937 gen(5);
        hsd::deque<hsd::string> owned;
        std::deque<std::string> reference;

        for (int i = 0; i < 4000; ++i)
        {
            std::string text = std::string(20 + gen() % 20, 'a' + gen() % 26)
                + std::to_string(i);

            if (gen() % 2)
            {
                owned.push_front(hsd::string(text.c_str()));
                reference.push_front(text);
            }
            else
            {
                owned.push_back(hsd::string(text.c_str()));
                reference.push_back(text);
            }

            if (gen() % 5 == 0)
            {
                owned.pop_front();
                reference.pop_front();
            }
        }

        assert(owned.size() == reference.size());
        size_t index = 0;

        for (auto it = owned.begin(); it != owned.end(); ++it, ++index)
        {
            assert(std::string((*it).c_str()) == reference[index]);
        }
    }

    // Array constructors with sizes that are not powers of two, then wrap.
    {
        hsd::deque<int> from_list{{1, 2, 3, 4, 5}};
        std::deque<int> reference{1, 2, 3, 4, 5};

        for (int i = 0; i < 20; ++i)
        {
            from_list.push_front(-i);
            from_list.push_back(i);
            reference.push_front(-i);
            reference.push_back(i);
        }

        assert(from_list.size() == reference.size());
        size_t index = 0;

        for (auto it = from_list.begin(); it != from_list.end(); ++it, ++index)
        {
            assert(*it == reference[index]);
        }
    }

    return 0;
}
