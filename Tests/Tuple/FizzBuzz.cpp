#include <cassert>
#include <string>

static std::string classify(int value)
{
    const bool fizz = value % 3 == 0;
    const bool buzz = value % 5 == 0;

    if (fizz && buzz)
    {
        return "fizzbuzz";
    }
    if (fizz)
    {
        return "fizz";
    }
    if (buzz)
    {
        return "buzz";
    }
    return std::to_string(value);
}

int main()
{
    assert(classify(1) == "1");
    assert(classify(3) == "fizz");
    assert(classify(5) == "buzz");
    assert(classify(15) == "fizzbuzz");

    for (int value = 1; value <= 1000; ++value)
    {
        const auto result = classify(value);
        if (value % 15 == 0)
        {
            assert(result == "fizzbuzz");
        }
        else if (value % 3 == 0)
        {
            assert(result == "fizz");
        }
        else if (value % 5 == 0)
        {
            assert(result == "buzz");
        }
        else
        {
            assert(result == std::to_string(value));
        }
    }
}
