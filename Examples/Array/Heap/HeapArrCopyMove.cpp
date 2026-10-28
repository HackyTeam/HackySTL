// Title: Heap array copy and move
// Level: Intermediate
// Shows: deep copy, element-wise copy assignment, and move construction and assignment.

#include <Containers/HeapArray.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename Arr>
void print_all(const char* label, const Arr& values)
{
    hsd::print("{}:"_fmt, label);
    for (const auto& value : values)
    {
        hsd::print(" {}"_fmt, value);
    }
    hsd::println(""_fmt);
}

int main()
{
    hsd::heap_array<hsd::i32, 4> original{{1, 2, 3, 4}};

    // The copy constructor allocates new storage, so the copies are independent.
    hsd::heap_array<hsd::i32, 4> copy{original};
    original[0] = 100;
    print_all("original", original);
    print_all("copy", copy);

    // Copy assignment keeps the destination's storage and copies the values.
    hsd::heap_array<hsd::i32, 4> assigned;
    const hsd::i32* assigned_storage = assigned.data();
    assigned = original;
    hsd::println("same storage after copy assignment: {}"_fmt,
        assigned.data() == assigned_storage ? "true" : "false");
    print_all("assigned", assigned);

    // Assigning a C array copies its values into the existing storage as well.
    assigned = {9, 8, 7, 6};
    print_all("assigned from C array", assigned);

    // Move construction steals the allocation, nothing is copied.
    const hsd::i32* original_storage = original.data();
    hsd::heap_array<hsd::i32, 4> moved{hsd::move(original)};
    hsd::println("moved took the storage: {}"_fmt,
        moved.data() == original_storage ? "true" : "false");
    print_all("moved", moved);
    // 'original' is empty now and must not be read, only destroyed.

    // Move assignment exchanges allocations, so 'moved' receives the old
    // storage of 'assigned', and both stay valid.
    assigned = hsd::move(moved);
    print_all("assigned after move", assigned);
    print_all("moved after move assignment", moved);

    return 0;
}
