#include <Allocator.hpp>
#include <Types.hpp>
#include <Pair.hpp>

int main()
{
    auto* data = hsd::mallocator::allocate_multiple<
        char>(12, 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0').unwrap();

    hsd::mallocator::deallocate(data);
}