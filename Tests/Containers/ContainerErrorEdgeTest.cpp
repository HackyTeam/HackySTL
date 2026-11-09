#include <cassert>
#include <string_view>

#include <Containers/Integer.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Wrappers/Pair.hpp>

// Error reporting of integer division and unordered map lookups, the zero-length and
// clamped paths of big integer splitting and the pair comparison, which are reached only
// with particular operands.
int main()
{
    // Dividing by zero reports the division error, with its message.
    hsd::integer<1024> _dividend{100};
    hsd::integer<1024> _zero{0};
    auto _division = _dividend.divide_with_reminder(_zero);
    assert(!_division.is_ok());
    assert(std::string_view{_division.unwrap_err().pretty_error()} == "Tried to divide by 0");

    // Multiplying large operands splits them into halves, which the long multiplication does
    // for operands of several words.
    assert((hsd::integer<1024>{123456789u} * hsd::integer<1024>{987654321u})
        == hsd::integer<1024>{121932631112635269ull});

    // The number of significant words is zero for zero.
    assert(_zero.num_size() == 0);

    // A missing key is reported as an invalid key, and the message names it.
    hsd::unordered_map<int, int> _map;
    _map.emplace(1, 10);
    auto _missing = _map.at(7);
    assert(!_missing.is_ok());
    assert(std::string_view{_missing.unwrap_err().pretty_error()} == "Tried to use an invalid key");
    assert(_map.at(1).is_ok() && _map.at(1).unwrap().get() == 10);

    // Erasing at a position outside the map is reported as an out-of-bounds access.
    auto _bad_position = _map.erase(_map.cend() + 5);
    assert(!_bad_position.is_ok());
    assert(std::string_view{_bad_position.unwrap_err().pretty_error()} == "Tried to access an element out of bounds");

    // Pairs with the same second element but a different first element are not equal.
    assert(!(hsd::pair<int, int>{1, 2} == hsd::pair<int, int>{3, 2}));
    // The braces hide the comma from the macro only inside the parentheses.
    assert((hsd::pair<int, int>{1, 2} == hsd::pair<int, int>{1, 2}));
    return 0;
}
