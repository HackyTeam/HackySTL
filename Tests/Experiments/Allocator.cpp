#include <Allocator.hpp>
#include <Types.hpp>
#include <Pair.hpp>

int main()
{
    auto* data = hsd::mallocator::allocate_multiple<
        char>(12, "hello world").unwrap();

    hsd::mallocator::deallocate(data);
}