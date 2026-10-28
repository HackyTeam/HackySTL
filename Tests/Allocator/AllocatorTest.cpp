#include <assert.h>

#include <Base/Allocator.hpp>
#include <Containers/Deque.hpp>
#include <Containers/ForwardList.hpp>
#include <Containers/HeapArray.hpp>
#include <Containers/List.hpp>
#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/SStream.hpp>

struct alignas(64) aligned_value
{
    int value = 0;

    aligned_value(int initial_value = 0)
        : value{initial_value}
    {}
};

struct tracking_allocator
{
    int* allocations;
    int* deallocations;
    int* constructions;
    int* destructions;
    int* selections;

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
        const tracking_allocator& lhs,
        const tracking_allocator& rhs)
    {
        return
            lhs.allocations == rhs.allocations &&
            lhs.deallocations == rhs.deallocations &&
            lhs.constructions == rhs.constructions &&
            lhs.destructions == rhs.destructions &&
            lhs.selections == rhs.selections;
    }
};

namespace hsd
{
    template <typename T>
    struct allocator_traits<tracking_allocator, T>
    {
        using allocator_type = tracking_allocator;
        using value_type = T;

        static auto allocate(allocator_type& alloc, usize size)
        {
            return alloc.template allocate<value_type>(size);
        }

        static auto deallocate(
            allocator_type& alloc, value_type* ptr, usize size)
        {
            return alloc.deallocate(ptr, size);
        }

        template <typename... Args>
        static auto* construct(
            allocator_type& alloc, value_type* ptr, Args&&... args)
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
            ++*alloc.selections;
            return alloc;
        }

        static bool equal(
            const allocator_type& lhs, const allocator_type& rhs)
        {
            return lhs == rhs;
        }
    };
}

using buffered_deque = hsd::deque<int, hsd::buffered_allocator>;
using buffered_forward_list = hsd::forward_list<
    int, hsd::buffered_allocator
>;
using buffered_heap_array = hsd::heap_array<
    int, 4, hsd::buffered_allocator
>;
using buffered_list = hsd::list<int, hsd::buffered_allocator>;
using buffered_string = hsd::basic_string<
    char, hsd::buffered_allocator
>;

static_assert(hsd::DefaultConstructible<hsd::vector<int>>);
static_assert(hsd::DefaultConstructible<hsd::deque<int>>);
static_assert(hsd::DefaultConstructible<hsd::list<int>>);
static_assert(hsd::DefaultConstructible<hsd::forward_list<int>>);
static_assert(hsd::DefaultConstructible<hsd::heap_array<int, 4>>);
static_assert(hsd::DefaultConstructible<hsd::string>);
static_assert(hsd::DefaultConstructible<hsd::unordered_map<int, int>>);
static_assert(hsd::DefaultConstructible<hsd::sstream>);

static_assert(!hsd::DefaultConstructible<hsd::buffered_vector<int>>);
static_assert(!hsd::DefaultConstructible<buffered_deque>);
static_assert(!hsd::DefaultConstructible<buffered_list>);
static_assert(!hsd::DefaultConstructible<buffered_forward_list>);
static_assert(!hsd::DefaultConstructible<buffered_heap_array>);
static_assert(!hsd::DefaultConstructible<buffered_string>);
static_assert(!hsd::DefaultConstructible<hsd::buffered_umap<int, int>>);
static_assert(!hsd::DefaultConstructible<hsd::buffered_sstream>);

static void test_default_allocator_alignment()
{
    hsd::allocator alloc;
    using traits = hsd::allocator_traits<hsd::allocator, aligned_value>;

    auto empty_allocation = traits::allocate(alloc, 0);
    assert(empty_allocation.is_ok());
    assert(empty_allocation.unwrap() == nullptr);

    auto* value = traits::allocate(alloc, 1).unwrap();
    assert(reinterpret_cast<hsd::uptr>(value) % alignof(aligned_value) == 0);

    traits::construct(alloc, value, 42);
    assert(value->value == 42);
    traits::destroy(alloc, value);
    traits::deallocate(alloc, value, 1).unwrap();
}

static void test_allocator_failure_paths()
{
    alignas(64) hsd::uchar bytes[128]{};
    hsd::buffered_allocator alloc{bytes, sizeof(bytes)};

    assert(!alloc.allocate<aligned_value>(1024).is_ok());

    auto* value = alloc.allocate<int>(1).unwrap();
    assert(alloc.deallocate(value + 1, 1).is_ok() == false);
    assert(alloc.deallocate(value, 1).is_ok());
}

static void test_buffer_alignment_and_coalescing()
{
    alignas(64) hsd::uchar bytes[1024]{};
    hsd::buffered_allocator alloc{bytes, sizeof(bytes)};

    auto* first = alloc.allocate<hsd::uchar>(128).unwrap();
    auto* second = alloc.allocate<hsd::uchar>(128).unwrap();
    auto* third = alloc.allocate<hsd::uchar>(128).unwrap();

    alloc.deallocate(second, 128).unwrap();
    alloc.deallocate(first, 128).unwrap();
    alloc.deallocate(third, 128).unwrap();

    auto* values = alloc.allocate<aligned_value>(8).unwrap();
    assert(reinterpret_cast<hsd::uptr>(values) % alignof(aligned_value) == 0);
    alloc.deallocate(values, 8).unwrap();
}

static void test_minimal_custom_allocator()
{
    int allocations = 0;
    int deallocations = 0;
    int constructions = 0;
    int destructions = 0;
    int selections = 0;
    tracking_allocator alloc{
        &allocations, &deallocations, &constructions, &destructions,
        &selections
    };

    {
        hsd::vector<int, tracking_allocator> values{alloc};

        for (int value = 0; value < 32; ++value)
        {
            values.push_back(value);
        }

        assert(values.back() == 31);
        values.erase_for(values.begin() + 16, values.end()).unwrap();
        assert(values.size() == 16);
    }

    {
        using tracking_map = hsd::unordered_map<
            int, int, hsd::hash<hsd::usize, int>, tracking_allocator
        >;
        tracking_map values{alloc};
        values.emplace(1, 2);
        values.emplace(3, 4);

        auto copy = values;
        assert(selections == 1);
        assert(copy[3] == 4);

        copy = copy;
        assert(copy.size() == 2);

        tracking_map assigned{alloc};
        assigned = values;
        assert(selections == 2);
        assert(assigned[1] == 2);
    }

    assert(allocations > 0);
    assert(allocations == deallocations);
    assert(constructions == destructions);
}

static void test_stateful_containers()
{
    alignas(64) hsd::uchar left_bytes[32768]{};
    alignas(64) hsd::uchar right_bytes[32768]{};
    hsd::buffered_allocator left_alloc{left_bytes, sizeof(left_bytes)};
    hsd::buffered_allocator right_alloc{right_bytes, sizeof(right_bytes)};
    const int initial_values[]{1, 2};
    int assigned_values[]{3, 4, 5};

    {
        hsd::buffered_vector<int> left{initial_values, left_alloc};
        hsd::buffered_vector<int> right{right_alloc};
        right = assigned_values;
        left = right;
        assert(left.size() == 3 && left[2] == 5);
    }

    {
        buffered_deque left{initial_values, left_alloc};
        buffered_deque right{right_alloc};
        right = assigned_values;
        left = right;
        assert(left.back() == 5);
    }

    {
        buffered_string left{"left", left_alloc};
        buffered_string right{"right", right_alloc};
        left = right;
        assert(left == "right");
        assert(left.get_allocator() == right_alloc);
    }

    {
        buffered_list left{initial_values, left_alloc};
        buffered_list right{right_alloc};
        right = assigned_values;
        left = right;
        assert(left.back() == 5);
    }

    {
        buffered_forward_list left{initial_values, left_alloc};
        buffered_forward_list right{right_alloc};
        right = assigned_values;
        left = right;
        assert(left.back() == 5);
    }

    {
        const int values[]{1, 2, 3, 4};
        buffered_heap_array array{values, left_alloc};
        assert(array[3] == 4);
    }

    {
        hsd::pair<int, int> values[]{{1, 2}, {3, 4}};
        hsd::buffered_umap<int, int> left{left_alloc};
        hsd::buffered_umap<int, int> map{values, right_alloc};

        for (int value = 5; value < 20; ++value)
        {
            map.emplace(value, value);
        }

        left = map;
        assert(map[3] == 4);
        assert(left[3] == 4);
        assert(left.get_allocator() == right_alloc);
    }

    {
        hsd::buffered_sstream stream{left_alloc};
        stream.push_back('x');
        assert(stream.back() == 'x');
    }
}

int main()
{
    test_default_allocator_alignment();
    test_allocator_failure_paths();
    test_buffer_alignment_and_coalescing();
    test_minimal_custom_allocator();
    test_stateful_containers();
}