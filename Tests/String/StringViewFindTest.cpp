#include <Containers/StringView.hpp>
#include <cassert>
#include <random>
#include <string_view>
#include <type_traits>
#include <vector>

// Compile-time evaluation uses the portable scan, not the run-time one.
constexpr hsd::string_view constant_view{"hello constexpr world"};
static_assert(constant_view.find(hsd::string_view{"world"}) == 16);
static_assert(constant_view.find(hsd::string_view{"nope"}) == hsd::string_view::npos);
static_assert(constant_view.find('c') == 6);
static_assert(constant_view.find('z') == hsd::string_view::npos);
static_assert(constant_view.find('o', 5) == 7);
static_assert(constant_view.find(hsd::string_view{"o"}, 5) == 7);

template <typename CharT, typename Rng>
static void test_find_against_std(Rng& rng, int rounds, int alphabet, int first_symbol)
{
    std::vector<CharT> haystack;
    std::vector<CharT> needle;

    auto pick = [&]
    {
        return static_cast<CharT>(first_symbol + static_cast<int>(rng() % alphabet));
    };

    for (int round = 0; round < rounds; ++round)
    {
        haystack.assign(rng() % 45, CharT{});
        needle.assign(1 + rng() % 5, CharT{});

        for (auto& symbol : haystack)
        {
            symbol = pick();
        }

        for (auto& symbol : needle)
        {
            symbol = pick();
        }

        if (!haystack.empty() && rng() % 2)
        {
            // Plant the needle so that hits are common.
            const auto at = static_cast<std::size_t>(rng() % (haystack.size() + 1));

            for (std::size_t i = 0; i < needle.size() && at + i < haystack.size(); ++i)
            {
                haystack[at + i] = needle[i];
            }
        }

        const hsd::basic_string_view<CharT> actual_haystack{haystack.data(), haystack.size()};
        const hsd::basic_string_view<CharT> actual_needle{needle.data(), needle.size()};
        const std::basic_string_view<CharT> expected_haystack{haystack.data(), haystack.size()};
        const std::basic_string_view<CharT> expected_needle{needle.data(), needle.size()};

        // Both libraries use the same npos value, so the results compare directly.
        for (std::size_t pos = 0; pos <= haystack.size(); ++pos)
        {
            assert(actual_haystack.find(actual_needle, pos)
                == expected_haystack.find(expected_needle, pos));
        }

        assert(actual_haystack.find(actual_needle) == expected_haystack.find(expected_needle));

        // The single-character overloads take a plain `char`.
        if constexpr (std::is_same_v<CharT, char>)
        {
            for (int attempt = 0; attempt < 4; ++attempt)
            {
                const CharT symbol = pick();

                for (std::size_t pos = 0; pos <= haystack.size(); ++pos)
                {
                    assert(actual_haystack.find(symbol, pos)
                        == expected_haystack.find(symbol, pos));
                }

                assert(actual_haystack.find(symbol) == expected_haystack.find(symbol));
            }
        }
    }
}

int main()
{
    std::mt19937_64 rng(2024);

    for (int alphabet : {1, 2, 3, 5, 26})
    {
        // Symbol 0 is the null character, an ordinary character here.
        test_find_against_std<char>(rng, 2000, alphabet, 0);
        test_find_against_std<unsigned char>(rng, 1000, alphabet, 0);

        test_find_against_std<char>(rng, 2000, alphabet, 1);
        test_find_against_std<unsigned char>(rng, 1000, alphabet, 1);

        // 0x7e..0x82+ crosses the signed/unsigned boundary of `char`.
        test_find_against_std<char>(rng, 2000, alphabet, 0x7e);
        test_find_against_std<unsigned char>(rng, 1000, alphabet, 0x7e);
    }

    for (int alphabet : {2, 4})
    {
        test_find_against_std<wchar_t>(rng, 1000, alphabet, 0);
        test_find_against_std<wchar_t>(rng, 1000, alphabet, 1);
        test_find_against_std<char16_t>(rng, 1000, alphabet, 0);
        test_find_against_std<char32_t>(rng, 1000, alphabet, 0);

        // Top of the 16-bit and Unicode ranges.
        test_find_against_std<char16_t>(rng, 1000, alphabet, 0xffff - alphabet);
        test_find_against_std<char32_t>(rng, 1000, alphabet, 0x10ffff - alphabet);
    }

    return 0;
}
