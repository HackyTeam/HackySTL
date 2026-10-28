#include <Allocator.hpp>
#include <Types.hpp>
#include <Pair.hpp>

#include "../Headers/Allocator.hpp"

int main()
{
    hsd::uchar buffer[256]{};
    hsd::uchar test[] = "Hello World!";
    hsd_test::buffered_allocator alloc = {buffer, 256u};

    auto* data = hsd::mallocator::allocate_multiple<
        char>(12, 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0').unwrap();

    hsd::mallocator::deallocate(data);

    char* data_arr[6];

    data_arr[0] = alloc.allocate<char>(4).unwrap();
    data_arr[1] = alloc.allocate<char>(4).unwrap();
    data_arr[2] = alloc.allocate<char>(4).unwrap();
    data_arr[3] = alloc.allocate<char>(4).unwrap();
    data_arr[4] = alloc.allocate<char>(4).unwrap();
    data_arr[5] = alloc.allocate<char>(4).unwrap();
    
    alloc.deallocate(data_arr[1], 0).unwrap();
    alloc.deallocate(data_arr[2], 0).unwrap();
    alloc.deallocate(data_arr[3], 0).unwrap();
    alloc.deallocate(data_arr[4], 0).unwrap();

    data = alloc.allocate<char>(sizeof(test)).unwrap();
    
    for (hsd::usize i = 0; auto chr : test)
    {
        data[i++] = chr;
    }

    alloc.deallocate(data_arr[0], 0).unwrap();
    alloc.deallocate(data_arr[5], 0).unwrap();
    alloc.deallocate(data, 0).unwrap();

    auto data2 = hsd_test::allocator::allocate<hsd::f128>(12).unwrap();
    hsd_test::allocator::deallocate(data2);
}