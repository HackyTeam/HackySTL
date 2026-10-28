#include <algorithm>
#include <array>
#include <any>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <forward_list>
#include <functional>
#include <list>
#include <memory>
#include <print>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <utility>
#include <vector>

#include <Base/Algorithm.hpp>
#include <Base/Math.hpp>
#include <Base/Unicode.hpp>
#include <Containers/Bitset.hpp>
#include <Containers/Deque.hpp>
#include <Containers/ForwardList.hpp>
#include <Containers/HeapArray.hpp>
#include <Containers/List.hpp>
#include <Containers/Range.hpp>
#include <Containers/StackArray.hpp>
#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Containers/Span.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Wrappers/Any.hpp>
#include <Wrappers/Functional.hpp>
#include <Wrappers/SharedPtr.hpp>
#include <Wrappers/Tuple.hpp>
#include <Wrappers/UniquePtr.hpp>
#include <Wrappers/Variant.hpp>
#include <Extra/Json.hpp>

namespace
{
    using clock_type = std::chrono::steady_clock;
    volatile std::uint32_t runtime_profile_seed = 0;

    std::uint32_t get_profile_seed()
    {
        return runtime_profile_seed % 1'000'000u;
    }

    struct profile_result
    {
        double median_ns_per_operation;
        double min_ns_per_operation;
        double max_ns_per_operation;
        std::uint64_t checksum;
    };

    template <typename T>
    inline void do_not_optimize(const T& value)
    {
#if defined(__GNUC__) || defined(__clang__)
        asm volatile("" : : "g"(value));
#elif defined(_MSC_VER)
        const T* volatile escaped = &value;
        (void)escaped;
#else
        const T* volatile escaped = &value;
        (void)escaped;
#endif
    }

    template <typename T>
    inline void preserve_allocation(const T* address)
    {
#if defined(__GNUC__) || defined(__clang__)
        asm volatile("" : : "g"(address) : "memory");
#else
        const T* volatile escaped = address;
        (void)escaped;
#endif
    }

    template <typename Workload>
    std::pair<double, std::uint64_t> measure_once(
        Workload& workload,
        std::size_t operation_count)
    {
        const auto start = clock_type::now();
        
        const auto checksum = workload();
        
        const auto stop = clock_type::now();
        
        do_not_optimize(checksum);
        
        return {
            std::chrono::duration<double, std::nano>(stop - start).count()
                / static_cast<double>(operation_count),
            checksum
        };
    }

    template <typename Setup, typename Operation>
    std::pair<double, std::uint64_t> measure_prepared_once(
        Setup& setup,
        Operation& operation,
        std::size_t operation_count)
    {
        auto state = setup();
        const auto start = clock_type::now();
        
        const auto checksum = operation(state);
        
        const auto stop = clock_type::now();
        
        do_not_optimize(checksum);
        
        return {
            std::chrono::duration<double, std::nano>(stop - start).count()
                / static_cast<double>(operation_count),
            checksum
        };
    }

    template <std::size_t SampleCount>
    profile_result summarize_samples(
        std::array<double, SampleCount> samples,
        std::uint64_t checksum)
    {
        std::ranges::sort(samples);
        
        return {
            samples[SampleCount / 2],
            samples.front(),
            samples.back(),
            checksum
        };
    }

    template <typename Workload>
    profile_result profile(Workload&& workload, std::size_t operation_count)
    {
        constexpr std::size_t warmup_count = 2;
        constexpr std::size_t sample_count = 11;
        
        std::array<double, sample_count> samples{};
        
        auto&& measured_workload = workload;
        
        std::uint64_t checksum = 0;

        for (std::size_t warmup = 0; warmup < warmup_count; ++warmup)
        {
            const auto [elapsed, current_checksum] = measure_once(
                measured_workload, operation_count
            );
            
            (void)elapsed;
            
            assert(warmup == 0 || checksum == current_checksum);
            checksum = current_checksum;
        }

        for (auto& sample : samples)
        {
            const auto [elapsed, current_checksum] = measure_once(
                measured_workload, operation_count
            );
            
            assert(current_checksum == checksum);
            sample = elapsed;
        }

        return summarize_samples(samples, checksum);
    }

    template <typename Workload>
    void report_profile(
        const char* name,
        std::size_t operation_count,
        Workload&& workload)
    {
        const auto result = profile(
            std::forward<Workload>(workload), operation_count
        );
        
        std::println(
            "{}: {:.2f} ns/op [{:.2f}, {:.2f}], checksum={}",
            name,
            result.median_ns_per_operation,
            result.min_ns_per_operation,
            result.max_ns_per_operation,
            result.checksum
        );
    }

    template <typename ActualWorkload, typename StandardWorkload>
    void compare_profile(
        const char* name,
        std::size_t operation_count,
        ActualWorkload&& actual_workload,
        StandardWorkload&& standard_workload)
    {
        constexpr std::size_t warmup_count = 2;
        constexpr std::size_t sample_count = 11;
        
        std::array<double, sample_count> actual_samples{};
        std::array<double, sample_count> standard_samples{};
        
        auto&& actual = actual_workload;
        auto&& standard = standard_workload;
        
        std::uint64_t actual_checksum = 0;
        std::uint64_t standard_checksum = 0;

        for (std::size_t warmup = 0; warmup < warmup_count; ++warmup)
        {
            const auto actual_result = measure_once(actual, operation_count);
            const auto standard_result = measure_once(standard, operation_count);
            
            assert(warmup == 0 || actual_checksum == actual_result.second);
            assert(warmup == 0 || standard_checksum == standard_result.second);
            
            actual_checksum = actual_result.second;
            standard_checksum = standard_result.second;
        }

        for (std::size_t sample = 0; sample < sample_count; ++sample)
        {
            const auto measure_actual = [&]
            {
                const auto [elapsed, checksum] = measure_once(actual, operation_count);
                
                assert(checksum == actual_checksum);
                actual_samples[sample] = elapsed;
            };
            const auto measure_standard = [&]
            {
                const auto [elapsed, checksum] = measure_once(standard, operation_count);
                
                assert(checksum == standard_checksum);
                standard_samples[sample] = elapsed;
            };

            if (sample % 2 == 0)
            {
                measure_actual();
                measure_standard();
            }
            else
            {
                measure_standard();
                measure_actual();
            }
        }

        const auto actual_result = summarize_samples(
            actual_samples, actual_checksum
        );
        
        const auto standard_result = summarize_samples(
            standard_samples, standard_checksum
        );
        
        assert(actual_result.checksum == standard_result.checksum);
        
        std::println(
            "{}: HackySTL={:.2f} ns/op [{:.2f}, {:.2f}], "
            "std={:.2f} ns/op [{:.2f}, {:.2f}], checksum={}",
            name,
            actual_result.median_ns_per_operation,
            actual_result.min_ns_per_operation,
            actual_result.max_ns_per_operation,
            standard_result.median_ns_per_operation,
            standard_result.min_ns_per_operation,
            standard_result.max_ns_per_operation,
            actual_result.checksum
        );
    }

    template <
        typename ActualSetup,
        typename ActualOperation,
        typename StandardSetup,
        typename StandardOperation
    >
    void compare_prepared_profile(
        const char* name,
        std::size_t operation_count,
        ActualSetup&& actual_setup,
        ActualOperation&& actual_operation,
        StandardSetup&& standard_setup,
        StandardOperation&& standard_operation)
    {
        constexpr std::size_t warmup_count = 2;
        constexpr std::size_t sample_count = 11;
        
        std::array<double, sample_count> actual_samples{};
        std::array<double, sample_count> standard_samples{};
        
        auto&& actual_prepare = actual_setup;
        auto&& actual_run = actual_operation;
        auto&& standard_prepare = standard_setup;
        auto&& standard_run = standard_operation;
        
        std::uint64_t actual_checksum = 0;
        std::uint64_t standard_checksum = 0;

        for (std::size_t warmup = 0; warmup < warmup_count; ++warmup)
        {
            const auto actual_result = measure_prepared_once(
                actual_prepare, actual_run, operation_count
            );
            
            const auto standard_result = measure_prepared_once(
                standard_prepare, standard_run, operation_count
            );
            
            assert(warmup == 0 || actual_checksum == actual_result.second);
            assert(warmup == 0 || standard_checksum == standard_result.second);
            
            actual_checksum = actual_result.second;
            standard_checksum = standard_result.second;
        }

        for (std::size_t sample = 0; sample < sample_count; ++sample)
        {
            const auto measure_actual = [&]
            {
                const auto [elapsed, checksum] = measure_prepared_once(
                    actual_prepare, actual_run, operation_count
                );
                
                assert(checksum == actual_checksum);
                actual_samples[sample] = elapsed;
            };
            const auto measure_standard = [&]
            {
                const auto [elapsed, checksum] = measure_prepared_once(
                    standard_prepare, standard_run, operation_count
                );
                
                assert(checksum == standard_checksum);
                standard_samples[sample] = elapsed;
            };

            if (sample % 2 == 0)
            {
                measure_actual();
                measure_standard();
            }
            else
            {
                measure_standard();
                measure_actual();
            }
        }

        const auto actual_result = summarize_samples(
            actual_samples, actual_checksum
        );
        
        const auto standard_result = summarize_samples(
            standard_samples, standard_checksum
        );
        
        assert(actual_result.checksum == standard_result.checksum);

        std::println(
            "{}: HackySTL={:.2f} ns/op [{:.2f}, {:.2f}], "
            "std={:.2f} ns/op [{:.2f}, {:.2f}], checksum={}",
            name,
            actual_result.median_ns_per_operation,
            actual_result.min_ns_per_operation,
            actual_result.max_ns_per_operation,
            standard_result.median_ns_per_operation,
            standard_result.min_ns_per_operation,
            standard_result.max_ns_per_operation,
            actual_result.checksum
        );
    }

    void profile_vector_append_and_iteration()
    {
        constexpr int item_count = 20000;
        compare_profile(
            "vector append + iteration",
            static_cast<std::size_t>(item_count) * 2,
            [] {
                hsd::vector<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::vector<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_string_append_and_iteration()
    {
        constexpr int char_count = 20000;
        compare_profile(
            "string append + iteration",
            static_cast<std::size_t>(char_count) * 2,
            [] {
                hsd::string value;

                for (int index = 0; index < char_count; ++index)
                {
                    value.push_back(static_cast<char>('a' + index % 26));
                }

                std::uint64_t checksum = 0;

                for (std::size_t index = 0; index < value.size(); ++index)
                {
                    checksum += static_cast<unsigned char>(value[index]);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::string value;

                for (int index = 0; index < char_count; ++index)
                {
                    value.push_back(static_cast<char>('a' + index % 26));
                }

                std::uint64_t checksum = 0;

                for (const auto character : value)
                {
                    checksum += static_cast<unsigned char>(character);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_unordered_map_insert_and_lookup()
    {
        constexpr int item_count = 5000;

        compare_prepared_profile(
            "unordered_map insert",
            item_count,
            [] { return hsd::unordered_map<int, int>{}; },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());
                
                for (int index = 0; index < item_count; ++index)
                {
                    const auto key = seed + (index * 7919) % item_count;
                    values[key] = key * 3 + 1;
                }

                return static_cast<std::uint64_t>(
                    values.size() + values[seed] + values[seed + item_count - 1]
                );
            },
            [] { return std::unordered_map<int, int>{}; },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());
                
                for (int index = 0; index < item_count; ++index)
                {
                    const auto key = seed + (index * 7919) % item_count;
                    values[key] = key * 3 + 1;
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.at(seed)
                    + values.at(seed + item_count - 1)
                );
            }
        );

        compare_prepared_profile(
            "unordered_map insert (reserved)",
            item_count,
            [] {
                hsd::unordered_map<int, int> values;
                values.reserve(item_count);
                return values;
            },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());
                
                for (int index = 0; index < item_count; ++index)
                {
                    const auto key = seed + (index * 7919) % item_count;
                    values[key] = key * 3 + 1;
                }

                return static_cast<std::uint64_t>(
                    values.size() + values[seed] + values[seed + item_count - 1]
                );
            },
            [] {
                std::unordered_map<int, int> values;
                values.reserve(item_count);
                return values;
            },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());

                for (int index = 0; index < item_count; ++index)
                {
                    const auto key = seed + (index * 7919) % item_count;
                    values[key] = key * 3 + 1;
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.at(seed)
                    + values.at(seed + item_count - 1)
                );
            }
        );

        compare_profile(
            "unordered_map hit lookup",
            item_count,
            [] {
                static const auto values = [] {
                    const auto seed = static_cast<int>(get_profile_seed());
                    hsd::unordered_map<int, int> populated;

                    for (int index = 0; index < item_count; ++index)
                    {
                        const auto key = seed + (index * 7919) % item_count;
                        populated[key] = key * 3 + 1;
                    }

                    return populated;
                }();

                const auto seed = static_cast<int>(get_profile_seed());
                std::uint64_t checksum = 0;

                for (int index = 0; index < item_count; ++index)
                {
                    const auto key = seed + (index * 7919) % item_count;
                    checksum += static_cast<std::uint64_t>(values[key]);
                }

                return checksum;
            },
            [] {
                static const auto values = [] {
                    const auto seed = static_cast<int>(get_profile_seed());
                    std::unordered_map<int, int> populated;

                    for (int index = 0; index < item_count; ++index)
                    {
                        const auto key = seed + (index * 7919) % item_count;
                        populated[key] = key * 3 + 1;
                    }

                    return populated;
                }();

                const auto seed = static_cast<int>(get_profile_seed());
                std::uint64_t checksum = 0;

                for (int index = 0; index < item_count; ++index)
                {
                    const auto key = seed + (index * 7919) % item_count;
                    checksum += static_cast<std::uint64_t>(values.at(key));
                }

                return checksum;
            }
        );
    }

    void profile_deque_end_operations()
    {
        constexpr int item_count = 10000;

        compare_prepared_profile(
            "deque push_back",
            item_count,
            [] { return hsd::deque<int>{}; },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(seed + index);
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.front() + values.back()
                );
            },
            [] { return std::deque<int>{}; },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(seed + index);
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.front() + values.back()
                );
            }
        );

        compare_prepared_profile(
            "deque push_back (HackySTL reserved; std growth)",
            item_count,
            [] {
                hsd::deque<int> values;
                values.reserve(item_count);
                return values;
            },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());
                
                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(seed + index);
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.front() + values.back()
                );
            },
            [] { return std::deque<int>{}; },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(seed + index);
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.front() + values.back()
                );
            }
        );

        compare_prepared_profile(
            "deque push_front",
            item_count,
            [] { return hsd::deque<int>{}; },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_front(seed + index);
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.front() + values.back()
                );
            },
            [] { return std::deque<int>{}; },
            [](auto& values) {
                const auto seed = static_cast<int>(get_profile_seed());
                
                for (int index = 0; index < item_count; ++index)
                {
                    values.push_front(seed + index);
                }

                return static_cast<std::uint64_t>(
                    values.size() + values.front() + values.back()
                );
            }
        );

        compare_profile(
            "deque sequential iteration",
            item_count,
            [] {
                static const auto values = [] {
                    const auto seed = static_cast<int>(get_profile_seed());
                    hsd::deque<int> populated;

                    for (int index = 0; index < item_count; ++index)
                    {
                        populated.push_back(seed + index);
                    }

                    return populated;
                }();

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                }

                return checksum;
            },
            [] {
                static const auto values = [] {
                    const auto seed = static_cast<int>(get_profile_seed());
                    std::deque<int> populated;

                    for (int index = 0; index < item_count; ++index)
                    {
                        populated.push_back(seed + index);
                    }

                    return populated;
                }();

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                }

                return checksum;
            }
        );
    }

    void profile_linked_lists()
    {
        constexpr int item_count = 5000;
        compare_profile(
            "list append + iteration",
            static_cast<std::size_t>(item_count) * 2,
            [] {
                hsd::list<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::list<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );

        compare_profile(
            "forward_list front insert + iteration",
            static_cast<std::size_t>(item_count) * 2,
            [] {
                hsd::forward_list<int> values;

                for (int index = item_count - 1; index >= 0; --index)
                {
                    values.push_front(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::forward_list<int> values;

                for (int index = item_count - 1; index >= 0; --index)
                {
                    values.push_front(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_sorting()
    {
        constexpr int item_count = 10000;
        compare_profile(
            "quick_sort",
            item_count,
            [] {
                hsd::vector<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back((index * 7919) % item_count);
                }

                hsd::quick_sort(values);
                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::vector<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back((index * 7919) % item_count);
                }

                std::ranges::sort(values);
                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );

        compare_profile(
            "radix_sort",
            item_count,
            [] {
                hsd::vector<unsigned> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(static_cast<unsigned>((index * 7919) % item_count));
                }

                hsd::radix_sort(values);
                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::vector<unsigned> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(static_cast<unsigned>((index * 7919) % item_count));
                }

                std::ranges::sort(values);
                std::uint64_t checksum = 0;

                for (const auto value : values)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_ranges()
    {
        constexpr int item_count = 10000;
        compare_profile(
            "filter + transform + sum",
            item_count,
            [] {
                hsd::vector<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values
                    | hsd::ranges::filter([](int item) { return item % 2 == 0; })
                    | hsd::ranges::transform([](int item) { return item * 3; }))
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::vector<int> values;

                for (int index = 0; index < item_count; ++index)
                {
                    values.push_back(index);
                }

                std::uint64_t checksum = 0;

                for (const auto value : values
                    | std::views::filter([](int item) { return item % 2 == 0; })
                    | std::views::transform([](int item) { return item * 3; }))
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_bit_operations()
    {
        constexpr std::size_t bit_count = 256;
        constexpr int rounds = 20000;
        compare_profile(
            "bitset set/test/count",
            static_cast<std::size_t>(rounds) * 2,
            [] {
                hsd::bitset<bit_count> bits;
                std::uint64_t checksum = 0;

                for (int round = 0; round < rounds; ++round)
                {
                    const auto index = static_cast<std::size_t>(round) % bit_count;
                    bits.flip(index);
                    checksum += bits.test(index).unwrap();
                    do_not_optimize(checksum);
                }

                return checksum + bits.count();
            },
            [] {
                std::bitset<bit_count> bits;
                std::uint64_t checksum = 0;

                for (int round = 0; round < rounds; ++round)
                {
                    const auto index = static_cast<std::size_t>(round) % bit_count;
                    bits.flip(index);
                    checksum += bits.test(index);
                    do_not_optimize(checksum);
                }

                return checksum + bits.count();
            }
        );
    }

    void profile_callable_erasure()
    {
        constexpr int call_count = 200000;
        compare_profile(
            "type-erased callable invocation",
            call_count,
            [] {
                hsd::function<int(int)> function = [](int value)
                {
                    return value * 3 + 1;
                };
                
                std::uint64_t checksum = 0;

                for (int index = 0; index < call_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(
                        function(index).unwrap()
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::function<int(int)> function = [](int value)
                {
                    return value * 3 + 1;
                };

                std::uint64_t checksum = 0;

                for (int index = 0; index < call_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(function(index));
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_type_erased_values()
    {
        constexpr int iteration_count = 20000;
        compare_profile(
            "any construct + cast",
            iteration_count,
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    hsd::any value{index};
                    checksum += static_cast<std::uint64_t>(
                        value.cast_to<int>().unwrap().get()
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    std::any value{index};
                    checksum += static_cast<std::uint64_t>(
                        *std::any_cast<int>(&value)
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_variant_visitation()
    {
        constexpr int iteration_count = 100000;
        compare_profile(
            "variant construct + visit",
            iteration_count,
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    hsd::variant<int, double> value{static_cast<int>(index)};
                    checksum += static_cast<std::uint64_t>(
                        value.visit([](const auto& item)
                        {
                            return static_cast<int>(item) + 1;
                        }).unwrap()
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    std::variant<int, double> value{index};
                    checksum += static_cast<std::uint64_t>(
                        std::visit([](const auto& item)
                        {
                            return static_cast<int>(item) + 1;
                        }, value)
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_pointer_ownership()
    {
        constexpr int iteration_count = 20000;
        compare_profile(
            "unique_ptr allocate + dereference",
            iteration_count,
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    auto value = hsd::make_unique<int>(static_cast<int>(index));
                    preserve_allocation(&*value);
                    checksum += static_cast<std::uint64_t>(*value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    auto value = std::make_unique<int>(index);
                    preserve_allocation(&*value);
                    checksum += static_cast<std::uint64_t>(*value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
        compare_profile(
            "shared_ptr allocate + copy + dereference",
            iteration_count,
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    auto value = hsd::make_safe_shared<int>(static_cast<int>(index));
                    preserve_allocation(&*value);
                    auto copy = value;
                    checksum += static_cast<std::uint64_t>(*copy);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    auto value = std::make_shared<int>(index);
                    preserve_allocation(&*value);
                    auto copy = value;
                    checksum += static_cast<std::uint64_t>(*copy);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_tuple_access()
    {
        constexpr int iteration_count = 1000000;
        compare_profile(
            "tuple indexed access",
            iteration_count,
            [] {
                const hsd::tuple<int, int, int> values{1, 2, 3};
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(
                        values.get<0>() + values.get<1>() + values.get<2>()
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                const std::tuple<int, int, int> values{1, 2, 3};
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(
                        std::get<0>(values) + std::get<1>(values) + std::get<2>(values)
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_math()
    {
        constexpr int iteration_count = 100000;
        compare_profile(
            "math sin + sqrt",
            iteration_count,
            [] {
                double checksum = 0.0;

                for (int index = 1; index <= iteration_count; ++index)
                {
                    const auto value = static_cast<double>(index) / 1000.0;
                    checksum += hsd::math::sin(value) + hsd::math::sqrt(value);
                    do_not_optimize(checksum);
                }

                return static_cast<std::uint64_t>(checksum * 1000.0);
            },
            [] {
                double checksum = 0.0;

                for (int index = 1; index <= iteration_count; ++index)
                {
                    const auto value = static_cast<double>(index) / 1000.0;
                    checksum += std::sin(value) + std::sqrt(value);
                    do_not_optimize(checksum);
                }

                return static_cast<std::uint64_t>(checksum * 1000.0);
            }
        );
    }

    void profile_fixed_arrays()
    {
        constexpr std::size_t item_count = 512;
        compare_profile(
            "stack_array fill + indexed access",
            item_count,
            [] {
                hsd::stack_array<int, item_count> values;

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    values[index] = static_cast<int>(index * 2);
                }

                std::uint64_t checksum = 0;

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(values[index]);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::array<int, item_count> values{};

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    values[index] = static_cast<int>(index * 2);
                }

                std::uint64_t checksum = 0;

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(values[index]);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
        compare_profile(
            "heap_array fill + indexed access",
            item_count,
            [] {
                hsd::heap_array<int, item_count> values;

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    values[index] = static_cast<int>(index * 2);
                }

                std::uint64_t checksum = 0;

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(values[index]);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::array<int, item_count> values{};

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    values[index] = static_cast<int>(index * 2);
                }

                std::uint64_t checksum = 0;

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    checksum += static_cast<std::uint64_t>(values[index]);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_json_parsing()
    {
        static constexpr char json[] = R"({"id":42,"values":[1,2,3],"name":"sample"})";
        constexpr int parse_count = 5000;
        report_profile(
            "HackySTL JSON lex + parse + access",
            parse_count,
            [] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < parse_count; ++index)
                {
                    hsd::JsonStream lexer;
                    lexer.lex(hsd::string_view{json, sizeof(json) - 1}).unwrap();
                    lexer.push_eot().unwrap();

                    hsd::JsonParser parser = lexer;
                    auto value = parser.parse_next().unwrap();
                    
                    checksum += static_cast<std::uint64_t>(
                        (*value)[hsd::string_view{"id"}].as_num<hsd::i32>().unwrap()
                    );
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_unicode_conversion()
    {
        constexpr int conversion_count = 100000;
        constexpr auto source = u8"Grüße 🌍";
        std::array<char32_t, 16> output{};
        report_profile(
            "HackySTL UTF-8 to UTF-32 conversion",
            conversion_count,
            [&] {
                std::uint64_t checksum = 0;

                for (int index = 0; index < conversion_count; ++index)
                {
                    output.fill(0);
                    hsd::unicode::convert_to(
                        output.data(), output.size(), source
                    ).unwrap();

                    checksum += static_cast<std::uint64_t>(output[0]);
                    do_not_optimize(checksum);

                    checksum += static_cast<std::uint64_t>(output[6]);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }

    void profile_non_owning_views()
    {
        constexpr std::size_t item_count = 2048;
        compare_profile(
            "span indexed traversal",
            item_count,
            [] {
                std::array<int, item_count> values{};

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    values[index] = static_cast<int>(index + 1);
                }

                hsd::span view{values};
                std::uint64_t checksum = 0;

                for (const auto value : view)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::array<int, item_count> values{};

                for (std::size_t index = 0; index < item_count; ++index)
                {
                    values[index] = static_cast<int>(index + 1);
                }

                std::span view{values};
                std::uint64_t checksum = 0;

                for (const auto value : view)
                {
                    checksum += static_cast<std::uint64_t>(value);
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );

        static const std::string text = []
        {
            std::string generated =
                "HackySTL profiling compares searchable string views consistently.";
            generated[0] = static_cast<char>(
                'A' + get_profile_seed() % 26u
            );
            return generated;
        }();

        static const std::string_view standard_view{text};
        static const hsd::string_view hacky_view{text.data(), text.size()};
        static constexpr std::string_view standard_hit{"consistently"};
        static constexpr hsd::string_view hacky_hit{"consistently"};
        static constexpr std::string_view standard_miss{"absent"};
        static constexpr hsd::string_view hacky_miss{"absent"};
        static const auto hit_position = standard_view.find(standard_hit);
        constexpr int search_count = 100000;

        compare_profile(
            "string_view find hit",
            search_count,
            [] {
                const auto seed = get_profile_seed();
                std::uint64_t checksum = 0;

                for (int index = 0; index < search_count; ++index)
                {
                    const auto start = static_cast<std::size_t>(
                        (index + seed) % (hit_position + 1)
                    );
                    checksum += hacky_view.find(hacky_hit, start);
                }

                return checksum;
            },
            [] {
                const auto seed = get_profile_seed();
                std::uint64_t checksum = 0;

                for (int index = 0; index < search_count; ++index)
                {
                    const auto start = static_cast<std::size_t>(
                        (index + seed) % (hit_position + 1)
                    );
                    checksum += standard_view.find(standard_hit, start);
                }

                return checksum;
            }
        );

        compare_profile(
            "string_view find miss",
            search_count,
            [] {
                const auto seed = get_profile_seed();
                std::uint64_t checksum = 0;

                for (int index = 0; index < search_count; ++index)
                {
                    const auto start = static_cast<std::size_t>(
                        (index * 13 + seed) % hacky_view.length()
                    );
                    checksum += hacky_view.find(hacky_miss, start);
                }

                return checksum;
            },
            [] {
                const auto seed = get_profile_seed();
                std::uint64_t checksum = 0;

                for (int index = 0; index < search_count; ++index)
                {
                    const auto start = static_cast<std::size_t>(
                        (index * 13 + seed) % standard_view.size()
                    );
                    checksum += standard_view.find(standard_miss, start);
                }

                return checksum;
            }
        );
    }

    void profile_random_engines()
    {
        constexpr int iteration_count = 200000;
        compare_profile(
            "mt19937 generation",
            iteration_count,
            [] {
                hsd::mt19937_32 engine{12345u};
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    checksum += engine.generate();
                    do_not_optimize(checksum);
                }

                return checksum;
            },
            [] {
                std::mt19937 engine{12345u};
                std::uint64_t checksum = 0;

                for (int index = 0; index < iteration_count; ++index)
                {
                    checksum += engine();
                    do_not_optimize(checksum);
                }

                return checksum;
            }
        );
    }
}

int main()
{
    runtime_profile_seed = static_cast<std::uint32_t>(
        clock_type::now().time_since_epoch().count() % 1'000'000
    );
    std::println(
        "Median of 11 steady-clock samples after warmup; timing is informational, "
        "not a pass/fail threshold."
    );
    profile_vector_append_and_iteration();
    profile_string_append_and_iteration();
    profile_unordered_map_insert_and_lookup();
    profile_deque_end_operations();
    profile_linked_lists();
    profile_sorting();
    profile_ranges();
    profile_bit_operations();
    profile_callable_erasure();
    profile_type_erased_values();
    profile_variant_visitation();
    profile_pointer_ownership();
    profile_tuple_access();
    profile_math();
    profile_random_engines();
    profile_fixed_arrays();
    profile_json_parsing();
    profile_unicode_conversion();
    profile_non_owning_views();
}
