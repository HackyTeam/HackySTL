#include <Containers/Vector.hpp>
#include <cassert>
#include <random>
#include <string>
#include <vector>

static long live = 0;

struct Pod
{
    int a;
    char b;
    double c;

    bool operator==(const Pod&) const = default;
};

// Owns heap memory and counts live instances, so leaks and double destruction show up.
struct Tracked
{
    std::string s;
    int v;

    Tracked(int x = 0)
        : s(30, 'a' + x % 26), v(x)
    {
        ++live;
    }

    Tracked(const Tracked& o)
        : s(o.s), v(o.v)
    {
        ++live;
    }

    Tracked(Tracked&& o) noexcept
        : s(std::move(o.s)), v(o.v)
    {
        ++live;
    }

    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) noexcept = default;

    ~Tracked()
    {
        --live;
    }

    bool operator==(const Tracked& o) const
    {
        return v == o.v && s == o.s;
    }
};

template <class E, class MK>
static void run(const char* name, MK mk, unsigned seed)
{
    std::mt19937 g(seed);

    for (int round = 0; round < 4; ++round)
    {
        long base = live;

        {
            hsd::vector<E> h;
            std::vector<E> s;

            for (long step = 0; step < 5000; ++step)
            {
                int op = g() % 100;

                if (op < 55)
                {
                    E x = mk(g());
                    h.push_back(x);
                    s.push_back(x);
                }
                else if (op < 65)
                {
                    int v = static_cast<int>(g());
                    h.emplace_back(mk(v));
                    s.emplace_back(mk(v));
                }
                else if (op < 75)
                {
                    if (!s.empty())
                    {
                        h.pop_back();
                        s.pop_back();
                    }
                }
                else if (op < 82)
                {
                    size_t n = static_cast<size_t>(g() % 3000);
                    h.resize(n);
                    s.resize(n);
                }
                else if (op < 88)
                {
                    size_t n = g() % 4000;
                    h.reserve(n);
                    s.reserve(n);
                }
                else if (op < 91)
                {
                    h.shrink_to_fit();
                    s.shrink_to_fit();
                }
                else if (op < 93)
                {
                    h.clear();
                    s.clear();
                }
                else if (op < 96)
                {
                    hsd::vector<E> c{h};
                    assert(c.size() == s.size());
                }

                assert(h.size() == s.size());

                if (step % 97 == 0 || step < 40)
                {
                    for (size_t i = 0; i < s.size(); ++i)
                    {
                        assert(h[i] == s[i]);
                    }
                }
            }

            for (size_t i = 0; i < s.size(); ++i)
            {
                assert(h[i] == s[i]);
            }
        }

        assert(live == base);   // every element constructed was destroyed
    }

    (void)name;
}

int main()
{
    run<Pod>(
        "Pod",
        [](unsigned x) { return Pod{static_cast<int>(x), static_cast<char>(x), x / 7.0}; },
        11);

    run<int>("int", [](unsigned x) { return static_cast<int>(x); }, 12);

    run<Tracked>(
        "Tracked(non-trivial)",
        [](unsigned x) { return Tracked(static_cast<int>(x % 1000)); },
        13);

    return 0;
}
