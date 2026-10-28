#pragma once

#include <cassert>
#include <print>
#include <string>

namespace test_support
{
    template <typename Actual, typename Expected>
    void expect_sequence(const Actual& actual, const Expected& expected)
    {
        auto actual_iter = actual.begin();
        auto expected_iter = expected.begin();
        std::size_t index = 0;

        for (; actual_iter != actual.end() && expected_iter != expected.end();
             ++actual_iter, ++expected_iter, ++index)
        {
            if (!(*actual_iter == *expected_iter))
            {
                std::println(
                    stderr, "sequence mismatch at index {}", index
                );
                assert(false);
            }
        }

        assert(actual_iter == actual.end());
        assert(expected_iter == expected.end());
    }

    template <typename Actual, typename Expected>
    void expect_map(const Actual& actual, const Expected& expected)
    {
        assert(actual.size() == expected.size());

        for (const auto& [key, value] : expected)
        {
            auto actual_value = actual.at(key);
            assert(actual_value.is_ok());
            assert(actual_value.unwrap().get() == value);
        }

        for (const auto& [key, value] : actual)
        {
            auto expected_iter = expected.find(key);
            assert(expected_iter != expected.end());
            assert(expected_iter->second == value);
        }
    }

    template <typename Actual>
    std::string to_string(const Actual& actual)
    {
        if (actual.length() == 0)
        {
            return {};
        }

        return {actual.data(), actual.length()};
    }

    template <typename Actual>
    void expect_string(const Actual& actual, const std::string& expected)
    {
        if (actual.length() != expected.size())
        {
            std::println(
                stderr,
                "string length mismatch: actual={} expected={}",
                actual.length(), expected.size()
            );
        }
        assert(actual.length() == expected.size());
        assert(test_support::to_string(actual) == expected);
    }
}