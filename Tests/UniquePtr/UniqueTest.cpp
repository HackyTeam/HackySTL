#include "../../cpp/UniquePtr.hpp"

struct base
{
    virtual void overload() = 0;
    virtual ~base() {}
};

struct derived
    : public base
{
    virtual void overload() override
    {}
};

int main()
{
    auto a = hsd::make_unique<int>(1);
    hsd::unique_ptr<int> b = hsd::move(a);
    hsd::unique_ptr<int[]> c = hsd::make_unique<int[]>(5);
    hsd::unique_ptr<base> d = hsd::make_unique<derived>();
}