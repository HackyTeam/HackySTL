#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>

#include <Filesystem/Path.hpp>

namespace fs = hsd::filesystem;
namespace std_fs = std::filesystem;

// Regression tests for renaming, listing after a rename, and creating directories.
// Everything happens under a fresh directory in the system temporary location, so
// the source tree is never touched.
namespace
{
    void write_text_file(const std::string& name)
    {
        std::FILE* file = std::fopen(name.c_str(), "wb");
        assert(file != nullptr);
        std::fputs("data", file);
        std::fclose(file);
    }

    bool exists(const std::string& name)
    {
        return std_fs::exists(name);
    }

    std::string text(const hsd::string& value)
    {
        return std::string{value.c_str(), value.length()};
    }

    bool list_contains(fs::path& directory, const std::string& name)
    {
        auto entries = directory.list();
        assert(entries.is_ok());
        for (auto& entry : entries.unwrap())
        {
            if (std_fs::path{text(entry.absolute_name())}.filename().string() == name)
            {
                return true;
            }
        }
        return false;
    }
}

int main()
{
    const std_fs::path base = std_fs::temp_directory_path() / "hsd_fs_rename_test";
    std_fs::remove_all(base);
    std_fs::create_directories(base);
    const std::string root = base.string();

    // Renaming a file keeps it in its directory and updates the path object.
    write_text_file(root + "/a.txt");
    fs::path file{(root + "/a.txt").c_str()};
    assert(file.rename("b.txt").is_ok());
    assert(!exists(root + "/a.txt"));
    assert(exists(root + "/b.txt"));
    assert(std_fs::path{text(file.absolute_name())} == std_fs::path{root + "/b.txt"});

    // A renamed directory keeps listing its contents through the same path object.
    std_fs::create_directories(base / "dir");
    write_text_file(root + "/dir/inside.txt");
    fs::path directory{(root + "/dir").c_str()};
    assert(directory.rename("moved").is_ok());
    assert(exists(root + "/moved/inside.txt"));
    assert(!exists(root + "/dir"));
    assert(list_contains(directory, "inside.txt"));

    write_text_file(root + "/moved/second.txt");
    assert(list_contains(directory, "second.txt"));

    // Listing through the same path object sees a file that was copied in and
    // then renamed, at every step.
    std_fs::create_directories(base / "watched");
    fs::path watched{(root + "/watched").c_str()};
    assert(list_contains(watched, "missing.txt") == false);
    write_text_file(root + "/watched/first.txt");
    assert(list_contains(watched, "first.txt"));
    assert(std_fs::path{text(watched.absolute_name())} == std_fs::path{root + "/watched"});
    fs::path first{(root + "/watched/first.txt").c_str()};
    assert(first.rename("second.txt").is_ok());
    assert(list_contains(watched, "second.txt"));
    assert(!list_contains(watched, "first.txt"));

    // Renaming onto an existing file replaces it, as rename(2) does.
    write_text_file(root + "/keep.txt");
    fs::path kept{(root + "/keep.txt").c_str()};
    assert(kept.rename("b.txt").is_ok());
    assert(!exists(root + "/keep.txt"));
    assert(exists(root + "/b.txt"));

    // Renaming a path that does not exist reports an error.
    fs::path missing{(root + "/nothing.txt").c_str()};
    assert(!missing.rename("other.txt").is_ok());

    // Creating a directory that already exists is not an error.
    fs::path parent{root.c_str()};
    assert(parent.create_directory("made").is_ok());
    assert(parent.create_directory("made").is_ok());
    assert(exists(root + "/made"));

    // A missing parent is an error, not a silent success.
    fs::path orphan{(root + "/no_such_parent").c_str()};
    assert(!orphan.create_directory("child").is_ok());
    assert(!exists(root + "/no_such_parent/child"));

    std_fs::remove_all(base);
}
