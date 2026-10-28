#include <cassert>

#include <Filesystem/Path.hpp>
#include <Support/TestSupport.hpp>

namespace fs = hsd::filesystem;

int main()
{
    fs::path current{"."};
    auto current_status = current.status();
    assert(current_status.exists());
    assert(current_status.is_directory().unwrap());
    assert(!current_status.is_regular_file().unwrap());

    auto entries = current.list();
    assert(entries.is_ok());
    assert(entries.unwrap().size() > 0);

    auto listed_again = current.list();
    assert(listed_again.is_ok());
    assert(listed_again.unwrap().size() > 0);

    fs::path test_source{"FSTest.cpp"};
    assert(test_source.status().exists());
    assert(test_source.status().is_regular_file().unwrap());
    assert(test_support::to_string(test_source.extension()) == ".cpp");

    fs::path missing{"./definitely_missing_hackystl_file"};
    assert(!missing.status().exists());

    auto parents = current.parents();
    assert(parents.size() > 0);
    assert(test_support::to_string(current.extension()).empty());

    fs::path copied{current};
    assert(copied == current);
    fs::path moved{hsd::move(copied)};
    assert(moved == current);
}
