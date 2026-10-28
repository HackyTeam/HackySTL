#include <Serialize/Io.hpp>

struct a
{
    hsd::i32 v;
    const char* key;

    hsd::i32 operator<=>(const a& rhs) const
    {
        return hsd::cstring::compare(key, rhs.key);
    }

    bool operator==(const a& rhs) const
    {
        return operator<=>(rhs) == 0; 
    }
};

int main()
{
    a val1{1, "your mom"};
    a val2{2, "this is"};

    using namespace hsd::format_literals;
    hsd::println("{}"_fmt, (val1 > val2) ? "true" : "false");
}