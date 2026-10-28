#include <cassert>

#include <Containers/StringView.hpp>
#include <Serialize/SStream.hpp>

using namespace hsd::format_literals;

// Regression test: an empty string view prints as nothing, without reading its
// null data pointer.
int main()
{
    hsd::static_sstream<32> buffer;
    buffer.write_data("<{}>"_fmt, hsd::string_view{});
    assert(buffer.size() == 2);
    assert(buffer.data()[0] == '<' && buffer.data()[1] == '>');

    hsd::static_sstream<32> filled;
    filled.write_data("[{}]"_fmt, hsd::string_view{"ab"});
    assert(filled.size() == 4);
}
