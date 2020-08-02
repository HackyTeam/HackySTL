#include "../../cpp/UniquePtr.hpp"
#include <memory>

int main()
{
    auto a = hsd::make_unique<int>(1);
    hsd::unique_ptr<int> b = hsd::move(a);
    hsd::unique_ptr<int[]> c = hsd::make_unique<int[]>(5);
}