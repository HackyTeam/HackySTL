#include <Containers/UnorderedMap.hpp>
#include <Base/Utility.hpp>
#include <cassert>

int main()
{
    hsd::unordered_map<int, int> source;
    source.emplace(1, 10);

    hsd::unordered_map<int, int> target{hsd::move(source)};
    assert(target.size() == 1);

    assert(source.size() == 0);
    source.emplace(2, 20);
    assert(source.size() == 1);
    assert(source.at(2).unwrap().get() == 20);
    assert(!source.at(1).is_ok());

    return 0;
}
