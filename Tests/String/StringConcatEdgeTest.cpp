#include <cassert>
#include <string>

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Support/TestSupport.hpp>

using namespace hsd::string_view_literals;

// Regression tests for concatenation: empty and unallocated operands, appends
// into spare capacity, and appends whose source lies in the destination.
int main()
{
    hsd::string unallocated;
    assert(unallocated.length() == 0);
    assert(test_support::to_string(unallocated + "x") == "x");
    assert(test_support::to_string("x" + unallocated) == "x");
    assert(test_support::to_string(unallocated + unallocated).empty());
    assert(test_support::to_string(unallocated + hsd::string_view{}).empty());
    assert(test_support::to_string(hsd::string_view{} + unallocated).empty());
    assert(test_support::to_string(unallocated + static_cast<const char*>(nullptr)).empty());

    hsd::string appended;
    appended += "";
    appended += hsd::string{};
    appended += hsd::string_view{};
    appended += static_cast<const char*>(nullptr);
    assert(appended.length() == 0);
    appended += "ab";
    assert(test_support::to_string(appended) == "ab");

    // Appending into spare capacity must update the length.
    hsd::string reserved{"abc"};
    reserved.reserve(20);
    reserved += "de";
    assert(reserved.length() == 5);
    assert(test_support::to_string(reserved) == "abcde");

    hsd::string reserved_view{"abc"};
    reserved_view.reserve(20);
    reserved_view += "de"_sv;
    assert(test_support::to_string(reserved_view) == "abcde");

    hsd::string reserved_string{"abc"};
    reserved_string.reserve(20);
    reserved_string += hsd::string{"de"};
    assert(test_support::to_string(reserved_string) == "abcde");

    // Appending a string to itself, through each overload.
    hsd::string self{"ab"};
    self.reserve(16);
    self += self;
    assert(test_support::to_string(self) == "abab");

    hsd::string tight{"xy"};
    tight += tight.c_str();
    assert(test_support::to_string(tight) == "xyxy");

    hsd::string view_self{"pq"};
    view_self.reserve(16);
    view_self += hsd::string_view{view_self.c_str(), view_self.length()};
    assert(test_support::to_string(view_self) == "pqpq");

    // Long incremental appends against std::string.
    hsd::string grown;
    std::string expected;
    for (int round = 0; round < 300; ++round)
    {
        const char piece[] = {static_cast<char>('a' + round % 26), 'z', 0};
        grown += piece;
        expected += piece;
        assert(grown.length() == expected.size());
    }
    assert(test_support::to_string(grown) == expected);
}
