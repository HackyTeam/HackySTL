// Title: Counting allocator
// Level: Advanced
// Shows: a custom allocator with an allocator_traits specialization under vector and unordered_map.

#include <Base/Allocator.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// A stateful allocator that counts everything through pointers, so every
// copy of it reports into the same totals.
struct counting_allocator
{
    hsd::usize* allocations;
    hsd::usize* deallocations;
    hsd::usize* constructions;
    hsd::usize* destructions;
    hsd::usize* copies;

    template <typename T>
    auto allocate(hsd::usize size)
    {
        if (size != 0) ++*allocations;
        return hsd::allocator::allocate<T>(size);
    }

    template <typename T>
    auto deallocate(T* ptr, hsd::usize size)
    {
        if (ptr != nullptr) ++*deallocations;
        return hsd::allocator::deallocate(ptr, size);
    }

    friend bool operator==(
        const counting_allocator& lhs, const counting_allocator& rhs)
    {
        return lhs.allocations == rhs.allocations;
    }
};

namespace hsd
{
    // Specializing allocator_traits customizes object lifetime and what
    // happens when a container holding the allocator is copied.
    template <typename T>
    struct allocator_traits<counting_allocator, T>
    {
        using allocator_type = counting_allocator;
        using value_type = T;

        static auto allocate(allocator_type& alloc, usize size)
        {
            return alloc.template allocate<value_type>(size);
        }

        static auto deallocate(allocator_type& alloc, value_type* ptr, usize size)
        {
            return alloc.deallocate(ptr, size);
        }

        template <typename... Args>
        static auto* construct(allocator_type& alloc, value_type* ptr, Args&&... args)
        {
            ++*alloc.constructions;
            return construct_at(ptr, forward<Args>(args)...);
        }

        static void destroy(allocator_type& alloc, value_type* ptr)
        {
            ++*alloc.destructions;
            ptr->~value_type();
        }

        static allocator_type select_on_container_copy_construction(
            const allocator_type& alloc)
        {
            ++*alloc.copies;
            return alloc;
        }

        static bool equal(const allocator_type& lhs, const allocator_type& rhs)
        {
            return lhs == rhs;
        }
    };
} // namespace hsd

int main()
{
    hsd::usize allocations = 0;
    hsd::usize deallocations = 0;
    hsd::usize constructions = 0;
    hsd::usize destructions = 0;
    hsd::usize copies = 0;
    counting_allocator alloc{
        &allocations, &deallocations, &constructions, &destructions, &copies
    };

    {
        hsd::vector<hsd::i32, counting_allocator> values{alloc};
        values.reserve(16);
        hsd::println("after reserve: allocations={}"_fmt, allocations);

        for (hsd::i32 value = 0; value < 16; ++value)
        {
            values.push_back(value);
        }

        hsd::println("after 16 pushes: allocations={} constructions={}"_fmt,
            allocations, constructions);
        hsd::println("vector holds {} values, last is {}"_fmt, values.size(), values.back());
    }

    hsd::println("vector gone: balanced={}"_fmt,
        (allocations == deallocations && constructions == destructions) ? "true" : "false");

    {
        using counting_map = hsd::unordered_map<
            hsd::i32, hsd::i32, hsd::hash<hsd::usize, hsd::i32>, counting_allocator
        >;

        counting_map squares{alloc};
        for (hsd::i32 key = 1; key <= 5; ++key)
        {
            squares.emplace(key, key * key);
        }

        // Copying the map asks the traits which allocator the copy should use.
        auto copy = squares;
        hsd::println("copy asked the traits: {}"_fmt, copies);
        hsd::println("copy[4]={} size={}"_fmt, copy[4], copy.size());
    }

    hsd::println("map gone: balanced={}"_fmt,
        (allocations == deallocations && constructions == destructions) ? "true" : "false");
    hsd::println("memory was used: {}"_fmt, allocations > 0 ? "true" : "false");

    return 0;
}
