#include <Containers/Vector.hpp>
#include <cassert>
#include <string>

static std::string make_name(int i)
{
    return std::string(40, 'a') + std::to_string(i);
}

int main()
{
    hsd::vector<std::string> values;

    // Enough pushes to reallocate several times, so every element is moved on growth.
    for (int i = 0; i < 100; ++i)
    {
        values.push_back(make_name(i));
    }

    assert(values.size() == 100);

    for (int i = 0; i < 100; ++i)
    {
        assert(values[i] == make_name(i));
    }

    values.push_back(std::string("tail"));
    assert(values.size() == 101 && values[100] == "tail");

    return 0;
}
