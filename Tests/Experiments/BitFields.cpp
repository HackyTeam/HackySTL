#include <Types.hpp>

struct block 
{
    hsd::u64 in_use : 1;
    hsd::usize size : 8 * sizeof(hsd::usize) - 1;
};

int main()
{
    constexpr hsd::usize size = sizeof(block);
}